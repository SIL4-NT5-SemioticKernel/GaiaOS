#!/usr/bin/env python3
import os
import tkinter as tk
from tkinter import ttk, messagebox

# Optional plotting stack
try:
    import numpy as np
    import matplotlib
    from matplotlib import pyplot as plt
    from matplotlib.colors import LinearSegmentedColormap
    from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg

    HAS_MPL = True
except Exception:
    HAS_MPL = False

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
# Plot helpers
# ---------------------------------------------------------------------------

def load_ssv_matrix(path: str):
    """
    Load a matrix-like SSV where the first column is an index and remaining
    columns are values. Returns a 2D numpy array or None if empty/missing.
    """
    if not HAS_MPL or not os.path.exists(path):
        return None

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
        return None

    if not rows:
        return None

    max_len = max(len(r) for r in rows)
    padded = []
    for r in rows:
        if len(r) < max_len:
            r = r + [float("nan")] * (max_len - len(r))
        padded.append(r)

    return np.array(padded, dtype=float)

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
            v = r[i] if i < len(r) else float("nan")
            series[i].append(v)

    return ticks, series


# ---------------------------------------------------------------------------
# Main UI
# ---------------------------------------------------------------------------

class GaiaConfigUI(tk.Tk):
    def __init__(self):
        super().__init__()

        self.title("GaiaOS Control / Status Console")
        self.geometry("1200x800")

        # auto-refresh toggles
        self.summary_auto_var     = tk.BooleanVar(value=False)
        self.io_auto_var          = tk.BooleanVar(value=False)
        self.status_auto_var      = tk.BooleanVar(value=False)
        self.devmap_auto_var      = tk.BooleanVar(value=False)
        self.projection_auto_var  = tk.BooleanVar(value=False)
        self.scores_auto_var      = tk.BooleanVar(value=False)
        self.trace_auto_var       = tk.BooleanVar(value=False)
        self.io_hist_auto_var     = tk.BooleanVar(value=False)


        # track engine tick delta on summary tab
        self._last_session_tick = None

        # shared colormap for heatmaps
        if HAS_MPL:
            self.heatmap_cmap = LinearSegmentedColormap.from_list(
                "gaia_colormap",
                [
                    (0.0,  "black"),
                    (0.25, "cyan"),
                    (0.5,  "purple"),
                    (0.75, "magenta"),
                    (1.0,  "lime"),
                ]
            )
        else:
            self.heatmap_cmap = None

        self._build_widgets()
        self.refresh_all()

    # --- Layout scaffold ---------------------------------------------------

    def _build_widgets(self):
        self.status_var = tk.StringVar(value="Ready.")
        
        self.nb = ttk.Notebook(self)
        self.nb.pack(fill=tk.BOTH, expand=True)

        self.summary_frame   = ttk.Frame(self.nb)
        self.io_frame        = ttk.Frame(self.nb)
        self.io_hist_frame   = ttk.Frame(self.nb)
        self.ctrl_frame      = ttk.Frame(self.nb)
        self.autoexec_frame  = ttk.Frame(self.nb)
        self.onion_frame     = ttk.Frame(self.nb)
        self.update_frame    = ttk.Frame(self.nb)
        self.status_frame    = ttk.Frame(self.nb)
        self.devmap_frame    = ttk.Frame(self.nb)
        self.proj_frame      = ttk.Frame(self.nb)
        self.scores_frame    = ttk.Frame(self.nb)
        self.trace_frame     = ttk.Frame(self.nb)

        # Order: put Status/Goals first as the friendly front tab
        self.nb.add(self.summary_frame,   text="Status / Goals")
        self.nb.add(self.io_frame,        text="I/O State")
        self.nb.add(self.io_hist_frame,   text="I/O History")
        self.nb.add(self.ctrl_frame,      text="Control Panel")
        self.nb.add(self.autoexec_frame,  text="Autoexec")
        self.nb.add(self.onion_frame,     text="Onion Snapshot")
        self.nb.add(self.update_frame,    text="Update Script")
        self.nb.add(self.status_frame,    text="System Status")
        self.nb.add(self.devmap_frame,    text="Deviation Map")
        self.nb.add(self.proj_frame,      text="Projection")
        self.nb.add(self.scores_frame,    text="Output Scores")
        self.nb.add(self.trace_frame,     text="Trace Metrics")

        self._build_summary_tab()
        self._build_io_tab()
        self._build_io_history_tab()
        self._build_ctrl_tab()
        self._build_autoexec_tab()
        self._build_onion_tab()
        self._build_update_tab()
        self._build_status_tab()
        self._build_devmap_tab()
        self._build_proj_tab()
        self._build_scores_tab()
        self._build_trace_tab()

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
            text="Open Trace Metrics",
            command=lambda: self.nb.select(self.trace_frame),
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

    # --- I/O History tab (line plots) -------------------------------------

    def _build_io_history_tab(self):
        top = ttk.Frame(self.io_hist_frame)
        top.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)

        ctrl = ttk.Frame(top)
        ctrl.pack(fill=tk.X, pady=(0, 5))

        ttk.Label(
            ctrl,
            text="Afferent/Efferent logs (afferent_log.ssv, efferent_log.ssv)"
        ).pack(side=tk.LEFT)

        io_hist_auto_chk = ttk.Checkbutton(
            ctrl,
            text="Auto-refresh",
            variable=self.io_hist_auto_var,
            command=self.on_io_hist_auto_toggle,
        )
        io_hist_auto_chk.pack(side=tk.RIGHT)

        ttk.Button(
            ctrl,
            text="Refresh now",
            command=self.refresh_io_history,
        ).pack(side=tk.RIGHT, padx=(0, 8))

        body = ttk.Frame(top)
        body.pack(fill=tk.BOTH, expand=True)

        self.io_hist_body = body

        if HAS_MPL:
            # Two stacked plots: afferents and efferents
            fig, (ax_aff, ax_eff) = plt.subplots(2, 1, sharex=True)
            fig.tight_layout()
            self.io_hist_fig = fig
            self.io_hist_ax_aff = ax_aff
            self.io_hist_ax_eff = ax_eff

            self.io_hist_canvas = FigureCanvasTkAgg(fig, master=body)
            self.io_hist_canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)
        else:
            # Text fallback if matplotlib not available
            self.io_hist_fig = None
            self.io_hist_ax_aff = None
            self.io_hist_ax_eff = None
            self.io_hist_canvas = None

            self.io_hist_text = tk.Text(body, wrap="none")
            yscroll = ttk.Scrollbar(
                body, orient="vertical", command=self.io_hist_text.yview
            )
            xscroll = ttk.Scrollbar(
                body, orient="horizontal", command=self.io_hist_text.xview
            )
            self.io_hist_text.configure(
                yscrollcommand=yscroll.set, xscrollcommand=xscroll.set
            )
            self.io_hist_text.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
            yscroll.pack(side=tk.RIGHT, fill=tk.Y)
            xscroll.pack(side=tk.BOTTOM, fill=tk.X)

        # initial draw
        self.refresh_io_history()

    def refresh_io_history(self):
        ticks_a, series_a = load_io_log(AFFERENT_LOG_FILE)
        ticks_e, series_e = load_io_log(EFFERENT_LOG_FILE)

        if HAS_MPL and self.io_hist_fig is not None:
            self.io_hist_fig.clf()
            self.io_hist_ax_aff = self.io_hist_fig.add_subplot(211)
            self.io_hist_ax_eff = self.io_hist_fig.add_subplot(212, sharex=self.io_hist_ax_aff)

            any_aff = False
            if ticks_a and series_a:
                for idx, s in enumerate(series_a):
                    self.io_hist_ax_aff.plot(ticks_a, s, label=f"A{idx}")
                self.io_hist_ax_aff.set_ylabel("Afferent")
                self.io_hist_ax_aff.legend(loc="upper right", fontsize="x-small")
                any_aff = True

            any_eff = False
            if ticks_e and series_e:
                for idx, s in enumerate(series_e):
                    self.io_hist_ax_eff.plot(ticks_e, s, label=f"E{idx}")
                self.io_hist_ax_eff.set_ylabel("Efferent")
                self.io_hist_ax_eff.set_xlabel("Tick")
                self.io_hist_ax_eff.legend(loc="upper right", fontsize="x-small")
                any_eff = True

            if not any_aff and not any_eff:
                self.io_hist_ax_aff.text(
                    0.5, 0.5,
                    "No I/O log data yet",
                    ha="center", va="center",
                    transform=self.io_hist_ax_aff.transAxes,
                )

            self.io_hist_canvas.draw()
            self.status_var.set("Refreshed I/O history.")
        else:
            # Text mode: show last 50 lines of each log
            self.io_hist_text.delete("1.0", tk.END)

            def summarize(path, label):
                if not os.path.exists(path):
                    self.io_hist_text.insert(
                        tk.END, f"[{label}] {path} not found\n\n"
                    )
                    return
                with open(path, "r", encoding="utf-8") as f:
                    lines = f.readlines()
                self.io_hist_text.insert(
                    tk.END, f"[{label}] {path} (last 50 lines):\n"
                )
                for line in lines[-50:]:
                    self.io_hist_text.insert(tk.END, line)
                self.io_hist_text.insert(tk.END, "\n")

            summarize(AFFERENT_LOG_FILE, "Afferent")
            summarize(EFFERENT_LOG_FILE, "Efferent")
            self.status_var.set("Refreshed I/O history (text mode).")

    def on_io_hist_auto_toggle(self):
        if self.io_hist_auto_var.get():
            self._schedule_io_hist_refresh()

    def _schedule_io_hist_refresh(self):
        if not self.io_hist_auto_var.get():
            return
        self.refresh_io_history()
        self.after(1000, self._schedule_io_hist_refresh)


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

    # --- Deviation Map tab (heatmap) --------------------------------------

    def _build_devmap_tab(self):
        top = ttk.Frame(self.devmap_frame)
        top.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)

        if not HAS_MPL:
            ttk.Label(top, text="matplotlib not available – deviation heatmap disabled").pack()
            self.devmap_fig = self.devmap_ax = self.devmap_canvas = None
            return

        ttk.Label(top, text=f"Deviation mapping: {DEVIATION_FILE}").pack(anchor="w")

        fig, ax = plt.subplots()
        self.devmap_fig = fig
        self.devmap_ax = ax
        self.devmap_canvas = FigureCanvasTkAgg(fig, master=top)
        self.devmap_canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)

        btn_frame = ttk.Frame(top)
        btn_frame.pack(fill=tk.X, pady=5)

        ttk.Button(btn_frame, text="Refresh", command=self.refresh_devmap).pack(side=tk.LEFT)

        dev_auto_chk = ttk.Checkbutton(
            btn_frame,
            text="Auto-refresh",
            variable=self.devmap_auto_var,
            command=self.on_devmap_auto_toggle,
        )
        dev_auto_chk.pack(side=tk.LEFT, padx=10)

    def refresh_devmap(self):
        if not HAS_MPL or self.devmap_fig is None:
            return

        data = load_ssv_matrix(DEVIATION_FILE)

        self.devmap_fig.clf()
        self.devmap_ax = self.devmap_fig.add_subplot(111)

        if data is None:
            self.devmap_ax.text(
                0.5, 0.5,
                "No deviation data",
                ha="center", va="center",
                transform=self.devmap_ax.transAxes,
            )
        else:
            masked = np.ma.masked_invalid(data)
            im = self.devmap_ax.imshow(masked, cmap=self.heatmap_cmap, aspect="auto")
            self.devmap_fig.colorbar(im, ax=self.devmap_ax, shrink=0.7)
            self.devmap_ax.set_xlabel("Afferent index")
            self.devmap_ax.set_ylabel("Row index")

        self.devmap_canvas.draw()
        self.status_var.set(f"Refreshed deviation map from {DEVIATION_FILE!r}.")

    def on_devmap_auto_toggle(self):
        if self.devmap_auto_var.get():
            self._schedule_devmap_refresh()

    def _schedule_devmap_refresh(self):
        if not self.devmap_auto_var.get():
            return
        self.refresh_devmap()
        self.after(1000, self._schedule_devmap_refresh)

    # --- Projection tab (heatmap) -----------------------------------------

    def _build_proj_tab(self):
        top = ttk.Frame(self.proj_frame)
        top.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)

        if not HAS_MPL:
            ttk.Label(top, text="matplotlib not available – projection heatmap disabled").pack()
            self.proj_fig = self.proj_ax = self.proj_canvas = None
            return

        ttk.Label(top, text=f"Projected trajectory: {PROJECTION_FILE}").pack(anchor="w")

        fig, ax = plt.subplots()
        self.proj_fig = fig
        self.proj_ax = ax
        self.proj_canvas = FigureCanvasTkAgg(fig, master=top)
        self.proj_canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)

        btn_frame = ttk.Frame(top)
        btn_frame.pack(fill=tk.X, pady=5)

        ttk.Button(btn_frame, text="Refresh", command=self.refresh_projection).pack(side=tk.LEFT)

        proj_auto_chk = ttk.Checkbutton(
            btn_frame,
            text="Auto-refresh",
            variable=self.projection_auto_var,
            command=self.on_projection_auto_toggle,
        )
        proj_auto_chk.pack(side=tk.LEFT, padx=10)

    def refresh_projection(self):
        if not HAS_MPL or self.proj_fig is None:
            return

        data = load_ssv_matrix(PROJECTION_FILE)

        self.proj_fig.clf()
        self.proj_ax = self.proj_fig.add_subplot(111)

        if data is None:
            self.proj_ax.text(
                0.5, 0.5,
                "No projection data",
                ha="center", va="center",
                transform=self.proj_ax.transAxes,
            )
        else:
            masked = np.ma.masked_invalid(data)
            im = self.proj_ax.imshow(masked, cmap=self.heatmap_cmap, aspect="auto")
            self.proj_fig.colorbar(im, ax=self.proj_ax, shrink=0.7)
            self.proj_ax.set_xlabel("Channel index")
            self.proj_ax.set_ylabel("Chrono index")

        self.proj_canvas.draw()
        self.status_var.set(f"Refreshed projection from {PROJECTION_FILE!r}.")

    def on_projection_auto_toggle(self):
        if self.projection_auto_var.get():
            self._schedule_projection_refresh()

    def _schedule_projection_refresh(self):
        if not self.projection_auto_var.get():
            return
        self.refresh_projection()
        self.after(1000, self._schedule_projection_refresh)

    # --- Output Scores tab (heatmap) --------------------------------------

    def _build_scores_tab(self):
        top = ttk.Frame(self.scores_frame)
        top.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)

        if not HAS_MPL:
            ttk.Label(top, text="matplotlib not available – scores heatmap disabled").pack()
            self.scores_fig = self.scores_ax = self.scores_canvas = None
            return

        ttk.Label(top, text=f"Output scores (Fn/SA/DM/RC/Chrg): {OUTPUT_SCORES_FILE}").pack(anchor="w")

        fig, ax = plt.subplots()
        self.scores_fig = fig
        self.scores_ax = ax
        self.scores_canvas = FigureCanvasTkAgg(fig, master=top)
        self.scores_canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)

        btn_frame = ttk.Frame(top)
        btn_frame.pack(fill=tk.X, pady=5)

        ttk.Button(btn_frame, text="Refresh", command=self.refresh_scores).pack(side=tk.LEFT)

        scores_auto_chk = ttk.Checkbutton(
            btn_frame,
            text="Auto-refresh",
            variable=self.scores_auto_var,
            command=self.on_scores_auto_toggle,
        )
        scores_auto_chk.pack(side=tk.LEFT, padx=10)

    def refresh_scores(self):
        if not HAS_MPL or self.scores_fig is None:
            return

        data = load_ssv_matrix(OUTPUT_SCORES_FILE)

        self.scores_fig.clf()
        self.scores_ax = self.scores_fig.add_subplot(111)

        if data is None:
            self.scores_ax.text(
                0.5, 0.5,
                "No output score data",
                ha="center", va="center",
                transform=self.scores_ax.transAxes,
            )
        else:
            masked = np.ma.masked_invalid(data)
            im = self.scores_ax.imshow(masked, cmap=self.heatmap_cmap, aspect="auto")
            self.scores_fig.colorbar(im, ax=self.scores_ax, shrink=0.7)
            self.scores_ax.set_xlabel("Score component (Fn/SA/DM/RC/Chrg)")
            self.scores_ax.set_ylabel("Efferent index")

        self.scores_canvas.draw()
        self.status_var.set(f"Refreshed output scores from {OUTPUT_SCORES_FILE!r}.")

    def on_scores_auto_toggle(self):
        if self.scores_auto_var.get():
            self._schedule_scores_refresh()

    def _schedule_scores_refresh(self):
        if not self.scores_auto_var.get():
            return
        self.refresh_scores()
        self.after(1000, self._schedule_scores_refresh)

    # --- Trace Metrics tab (line plots) -----------------------------------

    def _build_trace_tab(self):
        top = ttk.Frame(self.trace_frame)
        top.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)

        if not HAS_MPL:
            ttk.Label(top, text="matplotlib not available – metrics plot disabled").pack()
            self.trace_fig = self.trace_ax = self.trace_canvas = None
            return

        ttk.Label(top, text="Trace / node metrics over time (System_State_Files)").pack(anchor="w")

        fig, ax = plt.subplots()
        self.trace_fig = fig
        self.trace_ax = ax
        self.trace_canvas = FigureCanvasTkAgg(fig, master=top)
        self.trace_canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)

        btn_frame = ttk.Frame(top)
        btn_frame.pack(fill=tk.X, pady=5)

        ttk.Button(btn_frame, text="Refresh", command=self.refresh_trace_metrics).pack(side=tk.LEFT)

        trace_auto_chk = ttk.Checkbutton(
            btn_frame,
            text="Auto-refresh",
            variable=self.trace_auto_var,
            command=self.on_trace_auto_toggle,
        )
        trace_auto_chk.pack(side=tk.LEFT, padx=10)

    def refresh_trace_metrics(self):
        if not HAS_MPL or self.trace_fig is None:
            return

        self.trace_fig.clf()
        self.trace_ax = self.trace_fig.add_subplot(111)

        metric_files = [
            (TRACE_VALID_FILE,        "Valid traces"),
            (TRACE_NEARLY_VALID_FILE, "Nearly valid traces"),
            (TRACE_TOTAL_FILE,        "Total output traces"),
            (NODE_COUNT_FILE,         "Node count"),
            (BOREDOM_FILE,            "Bored (0/1)"),
        ]

        any_data = False
        for path, label in metric_files:
            xs, ys = load_ssv_xy(path)
            if xs and ys:
                self.trace_ax.plot(xs, ys, label=label)
                any_data = True

        if not any_data:
            self.trace_ax.text(
                0.5, 0.5,
                "No metric data yet",
                ha="center", va="center",
                transform=self.trace_ax.transAxes,
            )
        else:
            self.trace_ax.set_xlabel("Tick")
            self.trace_ax.set_ylabel("Value")
            self.trace_ax.legend(loc="best")

        self.trace_canvas.draw()
        self.status_var.set("Refreshed trace/node metrics.")

    def on_trace_auto_toggle(self):
        if self.trace_auto_var.get():
            self._schedule_trace_refresh()

    def _schedule_trace_refresh(self):
        if not self.trace_auto_var.get():
            return
        self.refresh_trace_metrics()
        self.after(1000, self._schedule_trace_refresh)

    # --- global refresh ----------------------------------------------------

    def refresh_all(self):
        self.refresh_summary()
        self.refresh_io()
        self.refresh_io_history()
        self.load_control_panel()
        self.load_autoexec()
        self.load_onion_snapshot()
        self.load_update_script()
        self.refresh_status()
        self.refresh_devmap()
        self.refresh_projection()
        self.refresh_scores()
        self.refresh_trace_metrics()


if __name__ == "__main__":
    app = GaiaConfigUI()
    app.mainloop()
