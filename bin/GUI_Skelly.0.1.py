#!/usr/bin/env python3
import os
import math
import tkinter as tk
from tkinter import ttk, messagebox

# ---------------------------------------------------------------------------
# Basic paths – tweak these to match your world
# ---------------------------------------------------------------------------

PINOUT_FILE          = "pinout.cfg"
CONTROL_PANEL_FILE   = "Control_Panel.ssv"
CONTROL_PANEL_FLAG   = "Control_Panel_Flag.ssv"   # engine watches this

STATUS_FILE          = os.path.join("System_State_Files", "status.ssv")
ONION_SNAPSHOT_FILE  = os.path.join("System_State_Files", "onion.ssv")
UPDATE_SCRIPT_FILE   = os.path.join("Scripts", "update.txt")
AUTOEXEC_FILE        = "autoexec.ssv"

# legacy files still used by engine; plotting is now driven via Skelly_Panels.ssv
DEVIATION_FILE       = os.path.join("System_State_Files", "deviation_mapping.ssv")
PROJECTION_FILE      = os.path.join("System_State_Files", "projection.ssv")
OUTPUT_SCORES_FILE   = os.path.join("System_State_Files", "output_scores.ssv")

TRACE_VALID_FILE         = os.path.join("System_State_Files", "trace_valid.ssv")
TRACE_NEARLY_VALID_FILE  = os.path.join("System_State_Files", "trace_nearly_valid.ssv")
TRACE_TOTAL_FILE         = os.path.join("System_State_Files", "trace_total_output.ssv")
NODE_COUNT_FILE          = os.path.join("System_State_Files", "node_count.ssv")
BOREDOM_FILE             = os.path.join("System_State_Files", "boredom.ssv")

AFFERENT_LOG_FILE  = os.path.join("System_State_Files", "afferent_log.ssv")
EFFERENT_LOG_FILE  = os.path.join("System_State_Files", "efferent_log.ssv")

SKELLY_PANELS_FILE = "Skelly_Panels.ssv"   # routing table for Under-The-Hood plots

# ---------------------------------------------------------------------------
# Utility helpers
# ---------------------------------------------------------------------------

def read_first_line(path: str) -> str:
    """Return first line of file or '' if missing/error."""
    try:
        with open(path, "r", encoding="utf-8") as f:
            line = f.readline()
        return line.strip()
    except FileNotFoundError:
        return ""
    except Exception as e:
        return f"<error: {e}>"

def read_whole_file(path: str) -> str:
    """Return file contents or '' (with comment) if missing."""
    try:
        with open(path, "r", encoding="utf-8") as f:
            return f.read()
    except FileNotFoundError:
        return ""
    except Exception as e:
        return f"# error reading {path}: {e}\n"

def write_whole_file(path: str, text: str) -> None:
    """Write text, creating parent dirs if needed."""
    parent = os.path.dirname(path)
    if parent:
        os.makedirs(parent, exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        f.write(text)

def touch_flag(path: str, value: str = "1\n") -> bool:
    """Write a simple flag (default '1\\n')."""
    try:
        write_whole_file(path, str(value))
        return True
    except Exception as e:
        messagebox.showerror("Flag Error", f"Failed to write flag {path}:\n{e}")
        return False

# ---------------------------------------------------------------------------
# Status parsing helpers
# ---------------------------------------------------------------------------

def parse_status_rows(path: str):
    """
    Parse status.ssv into a list of token lists.
    Skips blank lines and comment lines.
    """
    rows = []
    try:
        with open(path, "r", encoding="utf-8") as f:
            for raw in f:
                line = raw.strip()
                if not line:
                    continue
                if line.startswith("#") or line.startswith("//"):
                    continue
                if ";" in line:
                    parts = [p.strip() for p in line.split(";")]
                else:
                    parts = line.split()
                rows.append(parts)
    except FileNotFoundError:
        pass
    except Exception as e:
        rows = [[f"<error reading {path}: {e}>"]]
    return rows

def parse_status_struct(path: str):
    """
    Parse status.ssv into structured dicts for engine, afferents, efferents.

    Expected row format (recommended):
        DOMAIN NAME VALUE [INFO ...]

    We only care about the first three tokens.
    """
    engine = {}
    afferents = {}  # index -> {Value, Goal, Dev, ...}
    efferents = {}  # index -> {Value, ...}

    rows = parse_status_rows(path)
    for parts in rows:
        if len(parts) < 3:
            continue
        domain, name, value_str = parts[0], parts[1], parts[2]
        # try parse numeric, fallback to string
        try:
            if "." in value_str or "e" in value_str.lower():
                value = float(value_str)
            else:
                value = int(value_str)
        except Exception:
            value = value_str

        if domain == "ENGINE":
            engine[name] = value
        elif domain == "AFFERENT":
            # expected name like A0_Value / A1_Goal / A2_Dev
            if name.startswith("A") and "_" in name:
                idx_str, field = name.split("_", 1)
                try:
                    idx = int(idx_str[1:])
                except Exception:
                    continue
                d = afferents.setdefault(idx, {})
                d[field] = value
        elif domain == "EFFERENT":
            if name.startswith("E") and "_" in name:
                idx_str, field = name.split("_", 1)
                try:
                    idx = int(idx_str[1:])
                except Exception:
                    continue
                d = efferents.setdefault(idx, {})
                d[field] = value

    return engine, afferents, efferents

# ---------------------------------------------------------------------------
# Plot/data helpers (no matplotlib; everything is lists)
# ---------------------------------------------------------------------------

def load_ssv_matrix(path: str):
    """
    Load a matrix-like SSV where the first column is an index and remaining
    columns are values. Returns a list-of-lists [[v00, v01, ...], ...]
    or [] if empty/missing.
    """
    if not os.path.exists(path):
        return []

    rows = []
    try:
        with open(path, "r", encoding="utf-8") as f:
            for line in f:
                line = line.strip()
                if not line:
                    continue
                if line.startswith("#") or line.startswith("//"):
                    continue
                parts = line.split()
                if len(parts) <= 1:
                    continue
                try:
                    vals = [float(v) for v in parts[1:]]
                except ValueError:
                    continue
                rows.append(vals)
    except Exception:
        return []

    if not rows:
        return []

    # pad short rows with NaNs (as None) to keep consistent lengths
    max_len = max(len(r) for r in rows)
    padded = []
    for r in rows:
        if len(r) < max_len:
            r = r + [None] * (max_len - len(r))
        padded.append(r)
    return padded

def load_ssv_xy(path: str):
    """
    Load simple time-series from SSV file with at least two columns:
    first is x (tick/time), second is y (value). Returns (xs, ys).
    """
    xs, ys = [], []
    if not os.path.exists(path):
        return xs, ys
    try:
        with open(path, "r", encoding="utf-8") as f:
            for line in f:
                line = line.strip()
                if not line:
                    continue
                if line.startswith("#") or line.startswith("//"):
                    continue
                parts = line.split()
                if len(parts) < 2:
                    continue
                try:
                    x = float(parts[0])
                    y = float(parts[1])
                except ValueError:
                    continue
                xs.append(x)
                ys.append(y)
    except Exception:
        return [], []
    return xs, ys

def load_io_log(path: str):
    """
    Load an I/O log SSV of the form:
        Tick v0 v1 v2 ...
    Returns:
        ticks:  [t0, t1, ...]
        series: [[ch0(t0..), ch0(t1..), ...], [ch1(...)] ...]
    """
    if not os.path.exists(path):
        return [], []

    ticks = []
    rows = []
    try:
        with open(path, "r", encoding="utf-8") as f:
            for line in f:
                line = line.strip()
                if not line:
                    continue
                if line.startswith("#") or line.startswith("//"):
                    continue
                parts = line.split()
                if len(parts) < 2:
                    continue
                try:
                    t = float(parts[0])
                    vals = [float(v) for v in parts[1:]]
                except ValueError:
                    continue
                ticks.append(t)
                rows.append(vals)
    except Exception:
        return [], []

    if not rows:
        return [], []

    n_series = max(len(r) for r in rows)
    series = [[] for _ in range(n_series)]
    for r in rows:
        for i in range(n_series):
            v = r[i] if i < len(r) else None
            series[i].append(v)

    return ticks, series

# ---------------------------------------------------------------------------
# Under-The-Hood panel definitions
# ---------------------------------------------------------------------------

class PanelDef:
    """
    One line from Skelly_Panels.ssv:

        panel_id ; label ; kind ; files

    - panel_id: internal key (no spaces)
    - label:    what shows in the dropdown
    - kind:     "io_log", "matrix_rows", "xy", "xy_multi"
    - files:    comma-separated list of paths, meaning depends on kind
    """
    def __init__(self, panel_id, label, kind, files):
        self.panel_id = panel_id
        self.label = label
        self.kind = kind
        self.files = files  # list[str]

def load_panel_defs(path: str):
    """
    Load panel definitions from Skelly_Panels.ssv.
    Lines are ';' separated:

        panel_id ; label ; kind ; files

    'files' is comma-separated if more than one.
    """
    defs = []
    if not os.path.exists(path):
        return defs

    try:
        with open(path, "r", encoding="utf-8") as f:
            for raw in f:
                line = raw.strip()
                if not line:
                    continue
                if line.startswith("#") or line.startswith("//"):
                    continue
                if ";" in line:
                    parts = [p.strip() for p in line.split(";")]
                else:
                    parts = line.split()
                if len(parts) < 4:
                    continue
                panel_id = parts[0]
                label = parts[1]
                kind = parts[2]
                files_str = parts[3]
                files = [p.strip() for p in files_str.split(",") if p.strip()]
                defs.append(PanelDef(panel_id, label, kind, files))
    except Exception:
        return []
    return defs

# ---------------------------------------------------------------------------
# Simple Tkinter Canvas line-plot widget
# ---------------------------------------------------------------------------

class LinePlotCanvas(tk.Canvas):
    """
    Dumb, fast line plot on a Tkinter Canvas.

    - plot(x, series_list, x_label, y_label, series_labels):
        x: list[float]
        series_list: list[list[float or None]]
    """

    def __init__(self, master, bg="black", **kwargs):
        super().__init__(master, bg=bg, highlightthickness=0, **kwargs)

    def plot(self, x_data, series_list, x_label=None, y_label=None, series_labels=None, message=None):
        self.delete("all")

        # If there's a message (e.g. "No data"), show that and bail.
        if message is not None:
            self.update_idletasks()
            w = max(self.winfo_width(), 10)
            h = max(self.winfo_height(), 10)
            self.create_text(w // 2, h // 2, text=message, fill="white")
            return

        if not x_data or not series_list:
            self.update_idletasks()
            w = max(self.winfo_width(), 10)
            h = max(self.winfo_height(), 10)
            self.create_text(w // 2, h // 2, text="No data", fill="white")
            return

        # Make sure geometry is up to date on first draw
        self.update_idletasks()
        w = max(self.winfo_width(), 10)
        h = max(self.winfo_height(), 10)

        # Flatten all y values ignoring None
        ys_all = []
        for series in series_list:
            for v in series:
                if v is not None:
                    ys_all.append(v)

        if not ys_all:
            self.create_text(w // 2, h // 2, text="No numeric data", fill="white")
            return

        x_min, x_max = min(x_data), max(x_data)
        y_min, y_max = min(ys_all), max(ys_all)

        if x_min == x_max:
            x_min -= 1.0
            x_max += 1.0
        if y_min == y_max:
            y_min -= 1.0
            y_max += 1.0

        left, right = 40, 10
        top, bottom = 10, 25

        plot_w = max(w - left - right, 1)
        plot_h = max(h - top - bottom, 1)

        def to_screen(x, y):
            fx = (x - x_min) / (x_max - x_min) if x_max != x_min else 0.5
            fy = (y - y_min) / (y_max - y_min) if y_max != y_min else 0.5
            sx = left + fx * plot_w
            sy = top + (1.0 - fy) * plot_h
            return sx, sy

        # Axes
        self.create_line(left, top, left, top + plot_h, fill="white")      # y-axis
        self.create_line(left, top + plot_h, left + plot_w, top + plot_h, fill="white")  # x-axis

        default_colors = ["cyan", "magenta", "yellow", "lime", "white", "orange"]
        series_labels = series_labels or [f"S{i}" for i in range(len(series_list))]

        # Draw each series
        for idx, ys in enumerate(series_list):
            color = default_colors[idx % len(default_colors)]
            points = []
            for x, y in zip(x_data, ys):
                if y is None:
                    continue
                sx, sy = to_screen(x, y)
                points.extend([sx, sy])
            if len(points) >= 4:
                self.create_line(points, fill=color, width=1)

        # Labels
        if y_label:
            self.create_text(
                left // 2,
                top + plot_h // 2,
                text=y_label,
                angle=90,
                fill="white",
            )
        if x_label:
            self.create_text(
                left + plot_w // 2,
                top + plot_h + 15,
                text=x_label,
                fill="white",
            )

        # Very simple legend in the upper-left of plot area
        y_offset = 5
        for label, color in zip(series_labels, default_colors):
            self.create_text(
                left + 5,
                top + y_offset,
                text=label,
                fill=color,
                anchor="nw",
            )
            y_offset += 12

# ---------------------------------------------------------------------------
# Main UI
# ---------------------------------------------------------------------------

class GaiaConfigUI(tk.Tk):
    def __init__(self):
        super().__init__()

        self.title("GaiaOS Control / Status Console")
        self.geometry("1200x800")

        # auto-refresh toggles
        self.summary_auto_var    = tk.BooleanVar(value=False)
        self.io_auto_var         = tk.BooleanVar(value=False)
        self.status_auto_var     = tk.BooleanVar(value=False)
        self.underhood_auto_var  = tk.BooleanVar(value=False)

        # Under-The-Hood controls
        self.window_depth_var    = tk.IntVar(value=0)   # 0 = full history
        self.smooth_window_var   = tk.IntVar(value=1)   # 1 = no smoothing

        # track engine tick delta on summary tab
        self._last_session_tick  = None

        # panel defs for Under-The-Hood
        self.panel_defs          = load_panel_defs(SKELLY_PANELS_FILE)
        self.panel_defs_by_label = {p.label: p for p in self.panel_defs}
        self.underhood_panel_var = tk.StringVar(value=self.panel_defs[0].label if self.panel_defs else "")

        # max points after downsampling for plots
        self.max_points          = 1000

        self._build_widgets()
        self.refresh_all()

    # --- Layout scaffold ---------------------------------------------------

    def _build_widgets(self):
        self.status_var = tk.StringVar(value="Ready.")

        self.nb = ttk.Notebook(self)
        self.nb.pack(fill=tk.BOTH, expand=True)

        self.summary_frame    = ttk.Frame(self.nb)
        self.io_frame         = ttk.Frame(self.nb)
        self.ctrl_frame       = ttk.Frame(self.nb)
        self.autoexec_frame   = ttk.Frame(self.nb)
        self.onion_frame      = ttk.Frame(self.nb)
        self.update_frame     = ttk.Frame(self.nb)
        self.status_frame     = ttk.Frame(self.nb)
        self.underhood_frame  = ttk.Frame(self.nb)

        # Order: put Status/Goals first as the friendly front tab
        self.nb.add(self.summary_frame,   text="Status / Goals")
        self.nb.add(self.io_frame,        text="I/O State")
        self.nb.add(self.ctrl_frame,      text="Control Panel")
        self.nb.add(self.autoexec_frame,  text="Autoexec")
        self.nb.add(self.onion_frame,     text="Onion Snapshot")
        self.nb.add(self.update_frame,    text="Update Script")
        self.nb.add(self.status_frame,    text="System Status")
        self.nb.add(self.underhood_frame, text="Under The Hood")

        self._build_summary_tab()
        self._build_io_tab()
        self._build_ctrl_tab()
        self._build_autoexec_tab()
        self._build_onion_tab()
        self._build_update_tab()
        self._build_status_tab()
        self._build_underhood_tab()

        # Status bar
        status = ttk.Label(self, textvariable=self.status_var, anchor="w")
        status.pack(fill=tk.X, side=tk.BOTTOM)

    # --- Summary / Goals tab ----------------------------------------------

    def _build_summary_tab(self):
        top = ttk.Frame(self.summary_frame)
        top.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)

        # Engine state box
        engine_frame = ttk.LabelFrame(top, text="Engine State")
        engine_frame.pack(fill=tk.X, padx=5, pady=5)

        self.summary_engine_label = ttk.Label(
            engine_frame,
            text="Engine: unknown",
            font=("TkDefaultFont", 12, "bold")
        )
        self.summary_engine_label.pack(anchor="w", padx=5, pady=2)

        self.summary_tick_label = ttk.Label(engine_frame, text="Session tick: -")
        self.summary_tick_label.pack(anchor="w", padx=5)

        self.summary_proc_label = ttk.Label(engine_frame, text="Processor tick: -")
        self.summary_proc_label.pack(anchor="w", padx=5)

        self.summary_boredom_label = ttk.Label(engine_frame, text="Engagement: unknown")
        self.summary_boredom_label.pack(anchor="w", padx=5, pady=(0, 5))

        # Middle: afferent / efferent summaries side by side
        mid = ttk.Frame(top)
        mid.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)

        # Afferents
        aff_frame = ttk.LabelFrame(mid, text="Afferent Goals & State")
        aff_frame.grid(row=0, column=0, sticky="nsew", padx=(0, 5), pady=5)

        aff_cols = ("id", "value", "goal", "dev")
        self.summary_aff_tree = ttk.Treeview(
            aff_frame, columns=aff_cols, show="headings", height=12
        )
        for col, width, title in zip(
            aff_cols, (60, 100, 100, 100),
            ("Afferent", "Value", "Goal", "Deviation")
        ):
            self.summary_aff_tree.heading(col, text=title)
            self.summary_aff_tree.column(col, width=width, anchor="w", stretch=True)
        aff_vsb = ttk.Scrollbar(
            aff_frame, orient="vertical", command=self.summary_aff_tree.yview
        )
        self.summary_aff_tree.configure(yscrollcommand=aff_vsb.set)
        self.summary_aff_tree.grid(row=0, column=0, sticky="nsew")
        aff_vsb.grid(row=0, column=1, sticky="ns")
        aff_frame.rowconfigure(0, weight=1)
        aff_frame.columnconfigure(0, weight=1)

        # Efferents
        eff_frame = ttk.LabelFrame(mid, text="Efferent Outputs")
        eff_frame.grid(row=0, column=1, sticky="nsew", padx=(5, 0), pady=5)

        eff_cols = ("id", "value")
        self.summary_eff_tree = ttk.Treeview(
            eff_frame, columns=eff_cols, show="headings", height=12
        )
        for col, width, title in zip(
            eff_cols, (60, 120),
            ("Efferent", "Value")
        ):
            self.summary_eff_tree.heading(col, text=title)
            self.summary_eff_tree.column(col, width=width, anchor="w", stretch=True)
        eff_vsb = ttk.Scrollbar(
            eff_frame, orient="vertical", command=self.summary_eff_tree.yview
        )
        self.summary_eff_tree.configure(yscrollcommand=eff_vsb.set)
        self.summary_eff_tree.grid(row=0, column=0, sticky="nsew")
        eff_vsb.grid(row=0, column=1, sticky="ns")
        eff_frame.rowconfigure(0, weight=1)
        eff_frame.columnconfigure(0, weight=1)

        mid.rowconfigure(0, weight=1)
        mid.columnconfigure(0, weight=1)
        mid.columnconfigure(1, weight=1)

        # Bottom: controls / navigation
        btn_frame = ttk.Frame(top)
        btn_frame.pack(fill=tk.X, padx=5, pady=5)

        ttk.Button(btn_frame, text="Refresh", command=self.refresh_summary).pack(side=tk.LEFT)

        summary_auto_chk = ttk.Checkbutton(
            btn_frame,
            text="Auto-refresh",
            variable=self.summary_auto_var,
            command=self.on_summary_auto_toggle,
        )
        summary_auto_chk.pack(side=tk.LEFT, padx=10)

        ttk.Button(
            btn_frame,
            text="Open Control Panel",
            command=lambda: self.nb.select(self.ctrl_frame),
        ).pack(side=tk.LEFT, padx=5)

        ttk.Button(
            btn_frame,
            text="Open I/O State",
            command=lambda: self.nb.select(self.io_frame),
        ).pack(side=tk.LEFT, padx=5)

        ttk.Button(
            btn_frame,
            text="Open Under The Hood",
            command=lambda: self.nb.select(self.underhood_frame),
        ).pack(side=tk.LEFT, padx=5)

    def refresh_summary(self):
        engine, afferents, efferents = parse_status_struct(STATUS_FILE)

        # Engine state
        session_tick = engine.get("Session_Tick")
        proc_tick = engine.get("Processor_Tick")
        run_update = engine.get("Run_Update")
        exit_flag = engine.get("Exit_Flag")

        # engine text
        state_text = "Engine: unknown"
        if session_tick is not None:
            if self._last_session_tick is None:
                state_text = f"Engine: active (tick {session_tick})"
            else:
                if session_tick > self._last_session_tick:
                    state_text = "Engine: running"
                else:
                    state_text = "Engine: no tick change (idle or stopped)"
        self._last_session_tick = session_tick if session_tick is not None else self._last_session_tick

        if isinstance(run_update, (int, float)):
            if run_update:
                state_text += " | Updates: ON"
            else:
                state_text += " | Updates: OFF"

        if isinstance(exit_flag, (int, float)) and exit_flag:
            state_text += " | EXIT requested"

        self.summary_engine_label.config(text=state_text)

        self.summary_tick_label.config(
            text=f"Session tick: {session_tick if session_tick is not None else '-'}"
        )
        self.summary_proc_label.config(
            text=f"Processor tick: {proc_tick if proc_tick is not None else '-'}"
        )

        # Boredom from status file (NT4 Bored X)
        boredom_text = "Engagement: unknown"
        try:
            with open(STATUS_FILE, "r", encoding="utf-8") as f:
                for line in f:
                    line = line.strip()
                    if not line or line.startswith("#") or line.startswith("//"):
                        continue
                    parts = line.split()
                    if len(parts) >= 3 and parts[0] == "NT4" and parts[1] == "Bored":
                        try:
                            bored_val = float(parts[2])
                        except Exception:
                            bored_val = None
                        if bored_val is not None:
                            if bored_val >= 0.5:
                                boredom_text = "Engagement: BORED"
                            else:
                                boredom_text = "Engagement: exploring"
                        break
        except FileNotFoundError:
            boredom_text = "Engagement: (no status file)"
        self.summary_boredom_label.config(text=boredom_text)

        # Afferent table
        self.summary_aff_tree.delete(*self.summary_aff_tree.get_children())
        for idx in sorted(afferents.keys()):
            d = afferents[idx]
            val = d.get("Value", "")
            goal = d.get("Goal", "")
            dev = d.get("Dev", "")
            self.summary_aff_tree.insert(
                "", tk.END,
                values=(f"A{idx}", val, goal, dev)
            )

        # Efferent table
        self.summary_eff_tree.delete(*self.summary_eff_tree.get_children())
        for idx in sorted(efferents.keys()):
            d = efferents[idx]
            val = d.get("Value", "")
            self.summary_eff_tree.insert(
                "", tk.END,
                values=(f"E{idx}", val)
            )

        self.status_var.set("Summary / Goals updated.")

    def on_summary_auto_toggle(self):
        if self.summary_auto_var.get():
            self._schedule_summary_refresh()

    def _schedule_summary_refresh(self):
        if not self.summary_auto_var.get():
            return
        self.refresh_summary()
        self.after(1000, self._schedule_summary_refresh)

    # --- I/O tab -----------------------------------------------------------

    def _build_io_tab(self):
        top = ttk.Frame(self.io_frame)
        top.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)

        columns = ("mode", "desc", "path", "value")
        self.io_tree = ttk.Treeview(top, columns=columns, show="headings", height=20)

        for col, width in zip(columns, (60, 280, 320, 260)):
            self.io_tree.heading(col, text=col.capitalize())
            self.io_tree.column(col, width=width, anchor="w", stretch=True)

        vsb = ttk.Scrollbar(top, orient="vertical", command=self.io_tree.yview)
        self.io_tree.configure(yscrollcommand=vsb.set)

        self.io_tree.grid(row=0, column=0, sticky="nsew")
        vsb.grid(row=0, column=1, sticky="ns")

        top.rowconfigure(0, weight=1)
        top.columnconfigure(0, weight=1)

        btn_frame = ttk.Frame(self.io_frame)
        btn_frame.pack(fill=tk.X, padx=5, pady=5)

        ttk.Button(btn_frame, text="Refresh", command=self.refresh_io).pack(side=tk.LEFT)

        io_auto_chk = ttk.Checkbutton(
            btn_frame,
            text="Auto-refresh",
            variable=self.io_auto_var,
            command=self.on_io_auto_toggle,
        )
        io_auto_chk.pack(side=tk.LEFT, padx=10)

    def _parse_pinout(self):
        entries = []
        try:
            with open(PINOUT_FILE, "r", encoding="utf-8") as f:
                for raw_line in f:
                    raw = raw_line.rstrip("\n")
                    line = raw.strip()
                    if not line or line.startswith("//"):
                        continue

                    parts = line.split()
                    mode = parts[0]

                    try:
                        if mode == "A":
                            pins = parts[1:-1]
                            fpath = parts[-1]
                            desc = f"GPIO afferent pins={','.join(pins)}"
                        elif mode == "E":
                            pin = parts[1]
                            fpath = parts[2]
                            desc = f"Efferent GPIO pin={pin}"
                        elif mode == "A1W":
                            sensor_id = parts[1]
                            fpath = parts[2]
                            desc = f"1-Wire sensor {sensor_id}"
                        elif mode == "US":
                            trig, echo = parts[1], parts[2]
                            fpath = parts[3]
                            desc = f"Ultrasonic TRIG={trig}, ECHO={echo}"
                        elif mode == "PH":
                            addr, chan, fpath = parts[1], parts[2], parts[3]
                            desc = f"pH ADS1115 addr={addr}, chan={chan}"
                        else:
                            fpath = parts[-1]
                            desc = " ".join(parts[1:-1]) or "(no extra info)"
                    except Exception:
                        fpath = parts[-1] if len(parts) >= 2 else "(?)"
                        desc = f"(malformed) {raw}"

                    entries.append({
                        "mode": mode,
                        "desc": desc,
                        "path": fpath,
                        "raw": raw,
                    })
        except FileNotFoundError:
            pass
        return entries

    def refresh_io(self):
        self.io_tree.delete(*self.io_tree.get_children())
        entries = self._parse_pinout()
        for ent in entries:
            val = ""
            if ent["path"] not in ("", "(?)"):
                val = read_first_line(ent["path"])
            self.io_tree.insert(
                "", tk.END,
                values=(ent["mode"], ent["desc"], ent["path"], val)
            )
        self.status_var.set(f"Loaded {len(entries)} I/O entries from {PINOUT_FILE!r}.")

    def on_io_auto_toggle(self):
        if self.io_auto_var.get():
            self._schedule_io_refresh()

    def _schedule_io_refresh(self):
        if not self.io_auto_var.get():
            return
        self.refresh_io()
        self.after(1000, self._schedule_io_refresh)

    # --- Control Panel tab -------------------------------------------------

    def _build_ctrl_tab(self):
        top = ttk.Frame(self.ctrl_frame)
        top.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)

        ttk.Label(top, text=f"Control Panel file: {CONTROL_PANEL_FILE}").pack(anchor="w")

        text_frame = ttk.Frame(top)
        text_frame.pack(fill=tk.BOTH, expand=True)

        self.ctrl_text = tk.Text(text_frame, wrap="none", undo=True)
        vsb = ttk.Scrollbar(text_frame, orient="vertical",
                            command=self.ctrl_text.yview)
        self.ctrl_text.configure(yscrollcommand=vsb.set)

        self.ctrl_text.grid(row=0, column=0, sticky="nsew")
        vsb.grid(row=0, column=1, sticky="ns")

        text_frame.rowconfigure(0, weight=1)
        text_frame.columnconfigure(0, weight=1)

        btn_frame = ttk.Frame(top)
        btn_frame.pack(fill=tk.X, pady=5)

        ttk.Button(btn_frame, text="Reload", command=self.load_control_panel).pack(side=tk.LEFT)
        ttk.Button(btn_frame, text="Save",   command=self.save_control_panel).pack(side=tk.LEFT, padx=5)
        ttk.Button(btn_frame, text="Trigger flag", command=self.trigger_control_flag).pack(side=tk.LEFT, padx=5)

        self.ctrl_flag_label = ttk.Label(btn_frame, text="")
        self.ctrl_flag_label.pack(side=tk.LEFT, padx=10)

    def load_control_panel(self):
        text = read_whole_file(CONTROL_PANEL_FILE)
        self.ctrl_text.delete("1.0", tk.END)
        self.ctrl_text.insert("1.0", text)
        self.update_ctrl_flag_label()
        self.status_var.set(f"Loaded {CONTROL_PANEL_FILE!r}.")

    def save_control_panel(self):
        text = self.ctrl_text.get("1.0", tk.END)
        write_whole_file(CONTROL_PANEL_FILE, text)
        self.status_var.set(f"Saved {CONTROL_PANEL_FILE!r}.")

    def trigger_control_flag(self):
        if touch_flag(CONTROL_PANEL_FLAG, "1\n"):
            self.update_ctrl_flag_label()
            self.status_var.set(f"Set control panel flag {CONTROL_PANEL_FLAG!r}.")

    def update_ctrl_flag_label(self):
        if os.path.exists(CONTROL_PANEL_FLAG):
            val = read_first_line(CONTROL_PANEL_FLAG)
            self.ctrl_flag_label.config(
                text=f"Flag file {CONTROL_PANEL_FLAG}: {val!r}"
            )
        else:
            self.ctrl_flag_label.config(
                text=f"Flag file {CONTROL_PANEL_FLAG} does not exist."
            )

    # --- Autoexec tab ------------------------------------------------------

    def _build_autoexec_tab(self):
        top = ttk.Frame(self.autoexec_frame)
        top.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)

        ttk.Label(top, text=f"Autoexec script: {AUTOEXEC_FILE}").pack(anchor="w")

        text_frame = ttk.Frame(top)
        text_frame.pack(fill=tk.BOTH, expand=True)

        self.autoexec_text = tk.Text(text_frame, wrap="none", undo=True)
        vsb = ttk.Scrollbar(text_frame, orient="vertical",
                            command=self.autoexec_text.yview)
        self.autoexec_text.configure(yscrollcommand=vsb.set)

        self.autoexec_text.grid(row=0, column=0, sticky="nsew")
        vsb.grid(row=0, column=1, sticky="ns")

        text_frame.rowconfigure(0, weight=1)
        text_frame.columnconfigure(0, weight=1)

        btn_frame = ttk.Frame(top)
        btn_frame.pack(fill=tk.X, pady=5)

        ttk.Button(btn_frame, text="Reload", command=self.load_autoexec).pack(side=tk.LEFT)
        ttk.Button(btn_frame, text="Save",   command=self.save_autoexec).pack(side=tk.LEFT, padx=5)

    def load_autoexec(self):
        text = read_whole_file(AUTOEXEC_FILE)
        self.autoexec_text.delete("1.0", tk.END)
        self.autoexec_text.insert("1.0", text)
        self.status_var.set(f"Loaded {AUTOEXEC_FILE!r}.")

    def save_autoexec(self):
        text = self.autoexec_text.get("1.0", tk.END)
        write_whole_file(AUTOEXEC_FILE, text)
        self.status_var.set(f"Saved {AUTOEXEC_FILE!r}.")

    # --- Onion snapshot tab -----------------------------------------------

    def _build_onion_tab(self):
        top = ttk.Frame(self.onion_frame)
        top.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)

        ttk.Label(top, text=f"Onion snapshot file: {ONION_SNAPSHOT_FILE}").pack(anchor="w")

        text_frame = ttk.Frame(top)
        text_frame.pack(fill=tk.BOTH, expand=True)

        self.onion_text = tk.Text(text_frame, wrap="none", undo=True)
        vsb = ttk.Scrollbar(text_frame, orient="vertical",
                            command=self.onion_text.yview)
        self.onion_text.configure(yscrollcommand=vsb.set)

        self.onion_text.grid(row=0, column=0, sticky="nsew")
        vsb.grid(row=0, column=1, sticky="ns")

        text_frame.rowconfigure(0, weight=1)
        text_frame.columnconfigure(0, weight=1)

        btn_frame = ttk.Frame(top)
        btn_frame.pack(fill=tk.X, pady=5)

        ttk.Button(btn_frame, text="Reload", command=self.load_onion_snapshot).pack(side=tk.LEFT)

    def load_onion_snapshot(self):
        text = read_whole_file(ONION_SNAPSHOT_FILE)
        self.onion_text.delete("1.0", tk.END)
        self.onion_text.insert("1.0", text)
        self.status_var.set(f"Loaded {ONION_SNAPSHOT_FILE!r}.")

    # --- Update Script tab -------------------------------------------------

    def _build_update_tab(self):
        top = ttk.Frame(self.update_frame)
        top.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)

        ttk.Label(top, text=f"Update script (agent main): {UPDATE_SCRIPT_FILE}").pack(anchor="w")

        text_frame = ttk.Frame(top)
        text_frame.pack(fill=tk.BOTH, expand=True)

        self.update_text = tk.Text(text_frame, wrap="none", undo=True)
        vsb = ttk.Scrollbar(text_frame, orient="vertical",
                            command=self.update_text.yview)
        self.update_text.configure(yscrollcommand=vsb.set)

        self.update_text.grid(row=0, column=0, sticky="nsew")
        vsb.grid(row=0, column=1, sticky="ns")

        text_frame.rowconfigure(0, weight=1)
        text_frame.columnconfigure(0, weight=1)

        btn_frame = ttk.Frame(top)
        btn_frame.pack(fill=tk.X, pady=5)

        ttk.Button(btn_frame, text="Reload", command=self.load_update_script).pack(side=tk.LEFT)
        ttk.Button(btn_frame, text="Save",   command=self.save_update_script).pack(side=tk.LEFT, padx=5)

    def load_update_script(self):
        text = read_whole_file(UPDATE_SCRIPT_FILE)
        self.update_text.delete("1.0", tk.END)
        self.update_text.insert("1.0", text)
        self.status_var.set(f"Loaded {UPDATE_SCRIPT_FILE!r}.")

    def save_update_script(self):
        text = self.update_text.get("1.0", tk.END)
        write_whole_file(UPDATE_SCRIPT_FILE, text)
        self.status_var.set(f"Saved {UPDATE_SCRIPT_FILE!r}.")

    # --- System Status tab -------------------------------------------------

    def _build_status_tab(self):
        top = ttk.Frame(self.status_frame)
        top.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)

        ttk.Label(top, text=f"System status table: {STATUS_FILE}").pack(anchor="w")

        table_frame = ttk.Frame(top)
        table_frame.pack(fill=tk.BOTH, expand=True)

        self.status_tree = ttk.Treeview(table_frame, columns=(), show="headings", height=20)

        vsb = ttk.Scrollbar(table_frame, orient="vertical",
                            command=self.status_tree.yview)
        self.status_tree.configure(yscrollcommand=vsb.set)

        self.status_tree.grid(row=0, column=0, sticky="nsew")
        vsb.grid(row=0, column=1, sticky="ns")

        table_frame.rowconfigure(0, weight=1)
        table_frame.columnconfigure(0, weight=1)

        btn_frame = ttk.Frame(top)
        btn_frame.pack(fill=tk.X, pady=5)

        ttk.Button(btn_frame, text="Refresh", command=self.refresh_status).pack(side=tk.LEFT)

        auto_chk = ttk.Checkbutton(
            btn_frame,
            text="Auto-refresh",
            variable=self.status_auto_var,
            command=self.on_status_auto_toggle,
        )
        auto_chk.pack(side=tk.LEFT, padx=10)

    def refresh_status(self):
        rows = parse_status_rows(STATUS_FILE)

        if rows:
            num_cols = max(len(r) for r in rows)
        else:
            num_cols = 0

        columns = [f"c{i}" for i in range(num_cols)]
        self.status_tree["columns"] = columns

        for idx, col in enumerate(columns):
            self.status_tree.heading(col, text=f"Col {idx}")
            self.status_tree.column(col, width=100, anchor="w", stretch=True)

        self.status_tree.delete(*self.status_tree.get_children())

        for r in rows:
            padded = list(r) + [""] * (num_cols - len(r))
            self.status_tree.insert("", tk.END, values=padded)

        self.status_var.set(f"Loaded {len(rows)} status rows from {STATUS_FILE!r}.")

    def on_status_auto_toggle(self):
        if self.status_auto_var.get():
            self._schedule_status_refresh()

    def _schedule_status_refresh(self):
        if not self.status_auto_var.get():
            return
        self.refresh_status()
        self.after(1000, self._schedule_status_refresh)

    # --- Under-The-Hood tab -----------------------------------------------

    def _build_underhood_tab(self):
        top = ttk.Frame(self.underhood_frame)
        top.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)

        # --- top row: dropdown + refresh / auto
        ctrl = ttk.Frame(top)
        ctrl.pack(fill=tk.X, pady=(0, 5))

        ttk.Label(ctrl, text="Panel:").pack(side=tk.LEFT)

        self.underhood_combo = ttk.Combobox(
            ctrl,
            textvariable=self.underhood_panel_var,
            state="readonly",
            values=[p.label for p in self.panel_defs] if self.panel_defs else [],
            width=30,
        )
        self.underhood_combo.pack(side=tk.LEFT, padx=5)
        self.underhood_combo.bind("<<ComboboxSelected>>", lambda e: self.refresh_underhood())

        ttk.Button(ctrl, text="Refresh", command=self.refresh_underhood).pack(side=tk.LEFT, padx=5)

        auto_chk = ttk.Checkbutton(
            ctrl,
            text="Auto-refresh",
            variable=self.underhood_auto_var,
            command=self.on_underhood_auto_toggle,
        )
        auto_chk.pack(side=tk.LEFT, padx=10)

        # --- second row: window depth + smoothing
        ctrl2 = ttk.Frame(top)
        ctrl2.pack(fill=tk.X, pady=(0, 5))

        ttk.Label(ctrl2, text="Window (last N points, 0 = all):").pack(side=tk.LEFT)
        window_spin = ttk.Spinbox(
            ctrl2,
            from_=0,
            to=1000000,
            textvariable=self.window_depth_var,
            width=8,
        )
        window_spin.pack(side=tk.LEFT, padx=5)

        ttk.Label(ctrl2, text="Smoothing (moving average window, 1 = off):").pack(side=tk.LEFT, padx=(20, 0))
        smooth_spin = ttk.Spinbox(
            ctrl2,
            from_=1,
            to=1000,
            textvariable=self.smooth_window_var,
            width=6,
        )
        smooth_spin.pack(side=tk.LEFT, padx=5)

        # --- main plot area
        self.underhood_canvas = LinePlotCanvas(top)
        self.underhood_canvas.pack(fill=tk.BOTH, expand=True)

    def _apply_window_smoothing_downsample(self, x, series_list):
        """
        Given x and series_list, apply:
        - sliding window (last N points) if window_depth > 0
        - moving average smoothing (window size W) if W > 1
        - downsample to at most self.max_points points
        """
        if not x or not series_list:
            return x, series_list

        n = len(x)
        window = max(0, int(self.window_depth_var.get() or 0))
        smooth_w = max(1, int(self.smooth_window_var.get() or 1))

        # Sliding window
        if window > 0 and n > window:
            x = x[-window:]
            series_list = [s[-window:] for s in series_list]
            n = len(x)

        # Smoothing (simple trailing moving average; ignore None values)
        if smooth_w > 1:
            def smooth(series):
                out = []
                buf = []
                for v in series:
                    buf.append(v)
                    if len(buf) > smooth_w:
                        buf.pop(0)
                    # average non-None values
                    vals = [b for b in buf if b is not None]
                    if not vals:
                        out.append(None)
                    else:
                        out.append(sum(vals) / len(vals))
                return out
            series_list = [smooth(s) for s in series_list]

        # Downsample
        max_pts = max(10, int(self.max_points))
        if n > max_pts:
            step = math.ceil(n / max_pts)
            x = x[::step]
            series_list = [s[::step] for s in series_list]

        return x, series_list

    def _compute_panel_data(self, panel_def: PanelDef):
        """
        Convert a PanelDef into a dict:

        {
            "x": [...],
            "series": [[...], [...]],
            "x_label": str,
            "y_label": str,
            "labels": [str, ...],
        }
        """
        kind = panel_def.kind
        files = panel_def.files

        if kind == "io_log":
            if not files:
                return None
            ticks, series = load_io_log(files[0])
            labels = [f"ch{i}" for i in range(len(series))]
            return {
                "x": ticks,
                "series": series,
                "x_label": "Tick",
                "y_label": "Value",
                "labels": labels,
            }

        elif kind == "matrix_rows":
            if not files:
                return None
            mat = load_ssv_matrix(files[0])
            if not mat:
                return None
            n_cols = max(len(row) for row in mat)
            x = list(range(n_cols))
            series = mat  # each row is one series
            labels = [f"Row {i}" for i in range(len(series))]
            return {
                "x": x,
                "series": series,
                "x_label": "Index",
                "y_label": "Value",
                "labels": labels,
            }

        elif kind == "xy":
            if not files:
                return None
            xs, ys = load_ssv_xy(files[0])
            label = os.path.basename(files[0])
            return {
                "x": xs,
                "series": [ys],
                "x_label": "X",
                "y_label": "Y",
                "labels": [label],
            }

        elif kind == "xy_multi":
            if not files:
                return None
            all_x = None
            series_list = []
            labels = []
            for path in files:
                xs, ys = load_ssv_xy(path)
                if not xs or not ys:
                    continue
                if all_x is None:
                    all_x = xs
                else:
                    # Require same length; for now, we ignore mismatched series
                    if len(xs) != len(all_x):
                        continue
                series_list.append(ys)
                labels.append(os.path.basename(path))
            if all_x is None or not series_list:
                return None
            return {
                "x": all_x,
                "series": series_list,
                "x_label": "X",
                "y_label": "Value",
                "labels": labels,
            }

        # Unknown kind -> nothing
        return None

    def refresh_underhood(self):
        if not self.panel_defs:
            self.underhood_canvas.plot([], [], message="No panel definitions (Skelly_Panels.ssv missing or empty).")
            self.status_var.set("No Under-The-Hood panels defined.")
            return

        label = self.underhood_panel_var.get()
        panel_def = self.panel_defs_by_label.get(label)
        if panel_def is None:
            # if the selected label vanished or isn't valid, reset to first
            panel_def = self.panel_defs[0]
            self.underhood_panel_var.set(panel_def.label)

        data = self._compute_panel_data(panel_def)
        if not data or not data.get("x") or not data.get("series"):
            self.underhood_canvas.plot([], [], message=f"No data for panel: {panel_def.label}")
            self.status_var.set(f"No data for Under-The-Hood panel {panel_def.label!r}.")
            return

        x = data["x"]
        series = data["series"]
        x, series = self._apply_window_smoothing_downsample(x, series)

        self.underhood_canvas.plot(
            x,
            series,
            x_label=data.get("x_label", "X"),
            y_label=data.get("y_label", "Value"),
            series_labels=data.get("labels"),
        )
        self.status_var.set(f"Refreshed Under-The-Hood panel {panel_def.label!r}.")

    def on_underhood_auto_toggle(self):
        if self.underhood_auto_var.get():
            self._schedule_underhood_refresh()

    def _schedule_underhood_refresh(self):
        if not self.underhood_auto_var.get():
            return
        self.refresh_underhood()
        self.after(1000, self._schedule_underhood_refresh)

    # --- global refresh ----------------------------------------------------

    def refresh_all(self):
        self.refresh_summary()
        self.refresh_io()
        self.load_control_panel()
        self.load_autoexec()
        self.load_onion_snapshot()
        self.load_update_script()
        self.refresh_status()
        # Under-The-Hood: only draw once; auto-refresh can take over
        self.refresh_underhood()


if __name__ == "__main__":
    app = GaiaConfigUI()
    app.mainloop()
