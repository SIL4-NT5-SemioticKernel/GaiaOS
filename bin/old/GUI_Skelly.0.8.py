#!/usr/bin/env python3
import os
import math
import tkinter as tk
from tkinter import ttk, messagebox

# ---------------------------------------------------------------------------
# Basic paths – tweak these to match your world
# ---------------------------------------------------------------------------

SYSTEM_STATE_DIR     = "System_State_Files"
PINOUT_FILE          = "pinout.cfg"
CONTROL_PANEL_FILE   = "Control_Panel.ssv"
CONTROL_PANEL_FLAG   = "Control_Panel_Flag.ssv"   # engine watches this
UPDATE_FLAG_FILE     = "Update_Flag.ssv"   # engine watches this

STATUS_FILE          = os.path.join("System_State_Files", "status.ssv")
ONION_SNAPSHOT_FILE  = os.path.join("System_State_Files", "onion.ssv")
SCRIPTS_DIR          = "./scripts"
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


def _format_ssv_number(v):
    """Format numbers compactly for .ssv writes."""
    try:
        fv = float(v)
    except Exception:
        return "0"
    if math.isfinite(fv) and abs(fv - round(fv)) < 1e-12:
        return str(int(round(fv)))
    return f"{fv:g}"

def write_ssv_matrix(path: str, matrix, header_comment: str = None) -> None:
    """
    Write a whitespace-separated numeric matrix to `path` atomically.
    Preserves leading comment lines (# or //) already present in the file,
    unless `header_comment` is explicitly provided.
    """
    parent = os.path.dirname(path)
    if parent:
        os.makedirs(parent, exist_ok=True)

    leading = []
    if header_comment is not None:
        if header_comment:
            for line in header_comment.splitlines():
                leading.append(f"# {line}".rstrip() + "\n")
    else:
        try:
            with open(path, "r", encoding="utf-8") as f:
                for raw in f:
                    s = raw.strip()
                    if not s:
                        break
                    if s.startswith("#") or s.startswith("//"):
                        leading.append(raw if raw.endswith("\n") else raw + "\n")
                    else:
                        break
        except FileNotFoundError:
            pass
        except Exception:
            # If we can't read safely, just don't preserve.
            leading = []

    body_lines = []
    for row in (matrix or []):
        if row is None:
            body_lines.append("\n")
            continue
        nums = [_format_ssv_number(v) for v in row]
        body_lines.append(" ".join(nums).rstrip() + "\n")

    tmp = path + ".tmp"
    with open(tmp, "w", encoding="utf-8") as f:
        for l in leading:
            f.write(l)
        for l in body_lines:
            f.write(l)
    os.replace(tmp, path)

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
# Small widgets for the System Readout Panel tab
# ---------------------------------------------------------------------------

class LampCanvas(tk.Canvas):
    """A tiny indicator lamp: grey (off) / green (on)."""
    def __init__(self, master, diameter=16, **kwargs):
        w = diameter + 4
        h = diameter + 4
        super().__init__(
            master,
            width=w,
            height=h,
            bg=kwargs.pop("bg", master.cget("background") if hasattr(master, "cget") else "SystemButtonFace"),
            highlightthickness=0,
            **kwargs,
        )
        self._diameter = diameter
        self._circle = self.create_oval(2, 2, 2 + diameter, 2 + diameter, outline="black", fill="#777777")

    def set(self, on: bool):
        self.itemconfig(self._circle, fill="#2ecc71" if on else "#777777")


class VerticalGaugeCanvas(tk.Canvas):
    """A small vertical gauge with a red marker (value normalized 0..1)."""
    def __init__(self, master, width=26, height=70, grid_cols=6, grid_rows=12, **kwargs):
        super().__init__(
            master,
            width=width,
            height=height,
            bg="white",
            highlightthickness=1,
            highlightbackground="black",
            **kwargs,
        )
        self._w = width
        self._h = height
        self._grid_cols = max(2, int(grid_cols))
        self._grid_rows = max(2, int(grid_rows))
        self._marker = None
        self._draw_grid()

    def _draw_grid(self):
        self.delete("grid")
        # light grid
        for i in range(1, self._grid_cols):
            x = i * self._w / self._grid_cols
            self.create_line(x, 0, x, self._h, fill="#d0d0d0", tags="grid")
        for j in range(1, self._grid_rows):
            y = j * self._h / self._grid_rows
            self.create_line(0, y, self._w, y, fill="#d0d0d0", tags="grid")

    def set(self, value01: float):
        try:
            v = float(value01)
        except Exception:
            v = 0.0
        if not math.isfinite(v):
            v = 0.0
        v = max(0.0, min(1.0, v))
        y = (1.0 - v) * (self._h - 1)
        # draw a short horizontal marker
        if self._marker is None:
            self._marker = self.create_line(2, y, self._w - 2, y, fill="red", width=3)
        else:
            self.coords(self._marker, 2, y, self._w - 2, y)


class MiniTrendCanvas(tk.Canvas):
    """A small gridded trend plot (single series, red trace)."""
    def __init__(self, master, width=320, height=70, grid_cols=24, grid_rows=10, **kwargs):
        super().__init__(
            master,
            width=width,
            height=height,
            bg="white",
            highlightthickness=1,
            highlightbackground="black",
            **kwargs,
        )
        self._w = width
        self._h = height
        self._grid_cols = max(4, int(grid_cols))
        self._grid_rows = max(4, int(grid_rows))
        self._trace = None
        self._draw_grid()

    def _draw_grid(self):
        self.delete("grid")
        for i in range(1, self._grid_cols):
            x = i * self._w / self._grid_cols
            self.create_line(x, 0, x, self._h, fill="#d0d0d0", tags="grid")
        for j in range(1, self._grid_rows):
            y = j * self._h / self._grid_rows
            self.create_line(0, y, self._w, y, fill="#d0d0d0", tags="grid")

    def plot(self, values):
        # values: list[float|None]
        vals = [v for v in (values or []) if v is not None]
        if len(vals) < 2:
            if self._trace is not None:
                self.delete(self._trace)
                self._trace = None
            return

        vmin = min(vals)
        vmax = max(vals)
        if vmax == vmin:
            vmax = vmin + 1.0

        # x is index
        n = len(values)
        if n < 2:
            return

        def to_screen(i, v):
            x = i * (self._w - 1) / max(1, (n - 1))
            y = (1.0 - ((v - vmin) / (vmax - vmin))) * (self._h - 1)
            return x, y

        pts = []
        for i, v in enumerate(values):
            if v is None:
                continue
            x, y = to_screen(i, v)
            pts.extend([x, y])

        if len(pts) < 4:
            if self._trace is not None:
                self.delete(self._trace)
                self._trace = None
            return

        if self._trace is None:
            self._trace = self.create_line(pts, fill="red", width=3)
        else:
            self.coords(self._trace, *pts)

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
        
        # --- .ssv browser state ---
        self.current_ssv_dir = SYSTEM_STATE_DIR
        self.ssv_dir_label_var = tk.StringVar(
            value=f"Directory: {os.path.abspath(self.current_ssv_dir)}"
        )

        # panel defs for Under-The-Hood plots + System Readout Panels
        self._panels_mtime = os.path.getmtime(SKELLY_PANELS_FILE) if os.path.exists(SKELLY_PANELS_FILE) else None
        self.panel_defs = load_panel_defs(SKELLY_PANELS_FILE)
        self.panel_defs_by_label = {p.label: p for p in self.panel_defs}
        self.panel_defs_by_id = {p.panel_id: p for p in self.panel_defs}

        # Kinds that can be drawn on the Under-The-Hood line plot canvas
        self._plot_kinds = {"io_log", "matrix_rows", "xy", "xy_multi"}

        self.underhood_panel_defs = [p for p in self.panel_defs if p.kind in self._plot_kinds]
        self.underhood_defs_by_label = {p.label: p for p in self.underhood_panel_defs}
        self.underhood_panel_var = tk.StringVar(value=self.underhood_panel_defs[0].label if self.underhood_panel_defs else "")

        # System Readout Panels are defined in Skelly_Panels.ssv via kind == "readout".
        # The 'files' field is interpreted as a comma-separated list of panel_ids
        # that already exist in Skelly_Panels.ssv.
        self.readout_panel_defs = [p for p in self.panel_defs if p.kind == "readout"]
        self.readout_defs_by_label = {p.label: p for p in self.readout_panel_defs}
        self.readout_panel_var = tk.StringVar(value=self.readout_panel_defs[0].label if self.readout_panel_defs else "")
        self.readout_auto_var = tk.BooleanVar(value=False)

        # Readout "setpoint" editing state (writes into the goal matrix, when available)
        self.readout_channel_var = tk.IntVar(value=0)     # 0-based row index
        self.readout_step_var = tk.DoubleVar(value=1.0)
        self.readout_setpoint_var = tk.DoubleVar(value=0.0)

        self._readout_goal_path = None
        self._readout_goal_matrix = None
        self._readout_goal_row_count = 0
        # --- scripts tab state ---
        self.current_script_path = None
        self.scripts_combo_var   = tk.StringVar()
        # scripts_text will be created in _build_scripts_tab
        
        # --- I/O edit state (for the I/O form we'll add later) ---
        self.io_selected_path     = None
        self.io_selected_mode_var = tk.StringVar()
        self.io_selected_desc_var = tk.StringVar()
        self.io_selected_file_var = tk.StringVar()
        self.io_value_var         = tk.StringVar()

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
        self.ssv_frame        = ttk.Frame(self.nb)
        self.ctrl_frame       = ttk.Frame(self.nb)
        self.autoexec_frame   = ttk.Frame(self.nb)
        self.onion_frame      = ttk.Frame(self.nb)
        self.scripts_frame    = ttk.Frame(self.nb)   # NEW
        self.status_frame     = ttk.Frame(self.nb)
        self.underhood_frame  = ttk.Frame(self.nb)
        self.readout_frame    = ttk.Frame(self.nb)

        # Order: put Status/Goals first as the friendly front tab
        self.nb.add(self.summary_frame,   text="Status / Goals")
        self.nb.add(self.io_frame,        text="I/O State")
        self.nb.add(self.ssv_frame,       text="SSV Files")
        self.nb.add(self.scripts_frame,   text="Scripts")       # NEW
        self.nb.add(self.underhood_frame, text="Under The Hood")
        self.nb.add(self.readout_frame,    text="System Readout Panel")
        self.nb.add(self.ctrl_frame,      text="Control Panel")
        self.nb.add(self.autoexec_frame,  text="Autoexec")
        self.nb.add(self.onion_frame,     text="Onion Snapshot")
        self.nb.add(self.status_frame,    text="System Status")

        self._build_summary_tab()
        self._build_io_tab()
        self._build_ssv_tab()
        self._build_ctrl_tab()
        self._build_autoexec_tab()
        self._build_onion_tab()
        self._build_scripts_tab()     # NEW
        self._build_status_tab()
        self._build_underhood_tab()
        self._build_readout_tab()

        # Status bar
        status = ttk.Label(self, textvariable=self.status_var, anchor="w")
        status.pack(fill=tk.X, side=tk.BOTTOM)

    # --- SSV Files tab -------------------------------------------------------

    def _build_ssv_tab(self):
        top = ttk.Frame(self.ssv_frame)
        top.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        # Header row with directory + nav + delete
        hdr = ttk.Frame(top)
        hdr.pack(fill=tk.X, pady=3)

        ttk.Label(hdr, textvariable=self.ssv_dir_label_var).pack(
            side=tk.LEFT, padx=(5, 10)
        )

        ttk.Button(hdr, text="Up", command=self.ssv_go_up_dir).pack(
            side=tk.LEFT, padx=5
        )

        ttk.Button(hdr, text="Enter dir", command=self.ssv_enter_dir).pack(
            side=tk.LEFT, padx=5
        )

        ttk.Button(hdr, text="[DELETE_ALL]", command=self.delete_all_ssv).pack(
            side=tk.RIGHT, padx=10
        )



        # File selector
        mid = ttk.Frame(top)
        mid.pack(fill=tk.X, pady=3)
        ttk.Label(mid, text="Select file:").pack(side=tk.LEFT, padx=(5, 2))
        self.ssv_combo_var = tk.StringVar()
        self.ssv_combo = ttk.Combobox(mid, textvariable=self.ssv_combo_var, state="readonly", width=50)
        self.ssv_combo.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=5)
        ttk.Button(mid, text="Reload list", command=self.refresh_ssv_list).pack(side=tk.LEFT, padx=5)
        ttk.Button(mid, text="Load file", command=self.load_ssv_file).pack(side=tk.LEFT, padx=5)
        ttk.Button(mid, text="Save file", command=self.save_ssv_file).pack(side=tk.LEFT, padx=5)

        # Table view
        table_frame = ttk.Frame(top)
        table_frame.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        self.ssv_tree = ttk.Treeview(table_frame, show="headings")
        vsb = ttk.Scrollbar(table_frame, orient="vertical", command=self.ssv_tree.yview)
        self.ssv_tree.configure(yscrollcommand=vsb.set)
        self.ssv_tree.grid(row=0, column=0, sticky="nsew")
        vsb.grid(row=0, column=1, sticky="ns")
        table_frame.rowconfigure(0, weight=1)
        table_frame.columnconfigure(0, weight=1)
        self.ssv_tree.bind("<Double-1>", self._begin_edit_cell)

        # Initialize list
        self.refresh_ssv_list()

    def save_ssv_file(self):
        """Write the current table back into the selected .ssv file."""
        fname = self.ssv_combo_var.get()
        if not fname:
            messagebox.showwarning(".ssv Viewer", "No file selected.")
            return

        path = os.path.join(self.current_ssv_dir, fname)

        # Extract table contents
        rows = []
        for row_id in self.ssv_tree.get_children():
            values = self.ssv_tree.item(row_id, "values")
            if values:
                rows.append(" ".join(values))

        text = "\n".join(rows) + "\n"

        try:
            with open(path, "w", encoding="utf-8") as f:
                f.write(text)
        except Exception as e:
            messagebox.showerror(".ssv Viewer", f"Error saving to {path}:\n{e}")
            return

        self.status_var.set(f"Saved {fname} with {len(rows)} rows.")


    def refresh_ssv_list(self):
        """Refresh the dropdown list of SSV files and subdirectories."""

        base_dir = self.current_ssv_dir or SYSTEM_STATE_DIR
        base_dir = os.path.abspath(base_dir)

        if not os.path.isdir(base_dir):
            os.makedirs(base_dir, exist_ok=True)

        # Update label
        self.current_ssv_dir = base_dir
        self.ssv_dir_label_var.set(f"Directory: {base_dir}")

        names = os.listdir(base_dir)

        dirs = [
            name + "/"
            for name in names
            if os.path.isdir(os.path.join(base_dir, name))
        ]
        files = [
            name
            for name in names
            if name.endswith(".ssv")
            and os.path.isfile(os.path.join(base_dir, name))
        ]

        values = sorted(dirs) + sorted(files)

        self.ssv_combo["values"] = values
        if values:
            # Keep selection if possible
            cur = self.ssv_combo_var.get()
            if cur in values:
                self.ssv_combo_var.set(cur)
            else:
                self.ssv_combo_var.set(values[0])
        else:
            self.ssv_combo_var.set("")

        self.status_var.set(
            f"Found {len(files)} .ssv files and {len(dirs)} dirs in {base_dir}/"
        )
    def ssv_go_up_dir(self):
        """Go up one directory."""
        cur = os.path.abspath(self.current_ssv_dir or SYSTEM_STATE_DIR)

        parent = os.path.dirname(cur)

        self.current_ssv_dir = parent
        self.refresh_ssv_list()

    def ssv_enter_dir(self):
        """
        If the selected combo item is a directory (shown as 'name/'),
        descend into it.
        """
        base_dir = self.current_ssv_dir or SYSTEM_STATE_DIR
        base_dir = os.path.abspath(base_dir)

        sel = self.ssv_combo_var.get()
        if not sel:
            messagebox.showwarning(".ssv Viewer", "No directory selected.")
            return

        if not sel.endswith("/"):
            messagebox.showwarning(
                ".ssv Viewer",
                "Selected item is not a directory (it doesn't end with '/').",
            )
            return

        dirname = sel.rstrip("/")
        new_path = os.path.join(base_dir, dirname)

        if not os.path.isdir(new_path):
            messagebox.showerror(
                ".ssv Viewer",
                f"Directory no longer exists:\n{new_path}"
            )
            return

        self.current_ssv_dir = new_path
        self.refresh_ssv_list()

    def _begin_edit_cell(self, event):
        """Start editing the clicked cell."""
        region = self.ssv_tree.identify("region", event.x, event.y)
        if region != "cell":
            return

        row_id = self.ssv_tree.identify_row(event.y)
        col_id = self.ssv_tree.identify_column(event.x)
        if not row_id or not col_id:
            return

        # Column index (c0, c1, etc.)
        col_index = int(col_id.replace("#", "")) - 1
        old_value = self.ssv_tree.item(row_id, "values")[col_index]

        # Get the screen coords of the cell
        x, y, width, height = self.ssv_tree.bbox(row_id, col_id)

        # Create entry overlay
        self._edit_var = tk.StringVar(value=old_value)
        self._edit_entry = tk.Entry(self.ssv_tree, textvariable=self._edit_var)
        self._edit_entry.place(x=x, y=y, width=width, height=height)

        self._edit_entry.focus()
        self._edit_entry.bind("<Return>", lambda e: self._finish_edit_cell(row_id, col_index))
        self._edit_entry.bind("<Escape>", lambda e: self._cancel_edit_cell())
        self._edit_entry.bind("<FocusOut>", lambda e: self._finish_edit_cell(row_id, col_index))

    def _finish_edit_cell(self, row_id, col_index):
        """User committed (or unfocused) cell edit."""
        new_value = self._edit_var.get()
        values = list(self.ssv_tree.item(row_id, "values"))
        values[col_index] = new_value
        self.ssv_tree.item(row_id, values=values)

        self._cancel_edit_cell()

    def _cancel_edit_cell(self):
        """Remove editing widget."""
        if hasattr(self, "_edit_entry") and self._edit_entry:
            self._edit_entry.destroy()
            self._edit_entry = None


    def load_ssv_file(self):
        """Load the selected .ssv file into the table view."""
        fname = self.ssv_combo_var.get()
        if not fname:
            messagebox.showwarning(".ssv Viewer", "No file selected.")
            return

        fname = self.ssv_combo_var.get()
        if not fname:
            messagebox.showwarning(".ssv Viewer", "No file selected.")
            return

        if fname.endswith("/"):
            messagebox.showwarning(
                ".ssv Viewer",
                "Selected item is a directory. Use 'Enter dir' instead."
            )
            return

        base_dir = self.current_ssv_dir or SYSTEM_STATE_DIR
        path = os.path.join(base_dir, fname)

        if not os.path.isfile(path):
            messagebox.showerror(".ssv Viewer", f"File not found: {path}")
            return

        # Read rows
        try:
            with open(path, "r", encoding="utf-8") as f:
                lines = [l.strip() for l in f if l.strip() and not l.strip().startswith("#")]
        except Exception as e:
            messagebox.showerror(".ssv Viewer", f"Error reading {path}:\n{e}")
            return

        rows = [line.split() for line in lines]
        if not rows:
            messagebox.showinfo(".ssv Viewer", f"No data rows in {path}.")
            return

        num_cols = max(len(r) for r in rows)
        cols = [f"c{i}" for i in range(num_cols)]
        self.ssv_tree["columns"] = cols
        for c in cols:
            self.ssv_tree.heading(c, text=c)
            self.ssv_tree.column(c, width=100, anchor="w", stretch=True)

        self.ssv_tree.delete(*self.ssv_tree.get_children())
        for r in rows:
            self.ssv_tree.insert("", tk.END, values=r)

        self.status_var.set(f"Loaded {fname} with {len(rows)} rows.")

    def delete_all_ssv(self):
        """Delete all .ssv files in the current directory."""
        base_dir = os.path.abspath(self.current_ssv_dir or SYSTEM_STATE_DIR)

        if not messagebox.askyesno(
            ".ssv Viewer",
            f"Delete ALL .ssv files in this directory?\n\n{base_dir}"
        ):
            return

        deleted = 0
        if os.path.isdir(base_dir):
            for name in os.listdir(base_dir):
                if name.endswith(".ssv"):
                    try:
                        os.remove(os.path.join(base_dir, name))
                        deleted += 1
                    except Exception:
                        pass

        self.refresh_ssv_list()
        self.ssv_tree.delete(*self.ssv_tree.get_children())
        self.status_var.set(
            f"Deleted {deleted} .ssv files from {base_dir}/"
        )



    # --- Scripts tab -------------------------------------------------------

    def _build_scripts_tab(self):
        top = ttk.Frame(self.scripts_frame)
        top.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)

        # Top row: directory + combobox + buttons
        hdr = ttk.Frame(top)
        hdr.pack(fill=tk.X, pady=(0, 5))

        ttk.Label(hdr, text=f"Scripts directory: {SCRIPTS_DIR}").pack(side=tk.LEFT)

        ttk.Label(hdr, text="Script:").pack(side=tk.LEFT, padx=(15, 2))

        self.scripts_combo = ttk.Combobox(
            hdr,
            textvariable=self.scripts_combo_var,
            state="readonly",
            width=40,
            values=self._get_script_list(),
        )
        self.scripts_combo.pack(side=tk.LEFT, padx=2)
        self.scripts_combo.bind("<<ComboboxSelected>>", self.on_script_selected)

        ttk.Button(hdr, text="Rescan", command=self.rescan_scripts).pack(
            side=tk.LEFT, padx=5
        )
        ttk.Button(hdr, text="Reload", command=self.load_current_script).pack(
            side=tk.LEFT, padx=5
        )
        ttk.Button(hdr, text="Save", command=self.save_current_script).pack(
            side=tk.LEFT, padx=5
        )

        # Main text editor
        text_frame = ttk.Frame(top)
        text_frame.pack(fill=tk.BOTH, expand=True)

        self.scripts_text = tk.Text(text_frame, wrap="none", undo=True)
        vsb = ttk.Scrollbar(text_frame, orient="vertical",
                            command=self.scripts_text.yview)
        self.scripts_text.configure(yscrollcommand=vsb.set)

        self.scripts_text.grid(row=0, column=0, sticky="nsew")
        vsb.grid(row=0, column=1, sticky="ns")

        text_frame.rowconfigure(0, weight=1)
        text_frame.columnconfigure(0, weight=1)

        # Try to pick a default script on startup
        scripts = self._get_script_list()
        if scripts:
            self.scripts_combo["values"] = scripts
            self.scripts_combo_var.set(scripts[0])
            self.on_script_selected()

    def _get_script_list(self):
        """Return sorted list of regular files in SCRIPTS_DIR."""
        if not os.path.isdir(SCRIPTS_DIR):
            return []
        try:
            names = []
            for name in os.listdir(SCRIPTS_DIR):
                full = os.path.join(SCRIPTS_DIR, name)
                if os.path.isfile(full):
                    names.append(name)
            names.sort()
            return names
        except Exception:
            return []

    def rescan_scripts(self):
        scripts = self._get_script_list()
        self.scripts_combo["values"] = scripts
        if scripts and self.scripts_combo_var.get() not in scripts:
            # reset to first if previous selection vanished
            self.scripts_combo_var.set(scripts[0])
            self.on_script_selected()
        self.status_var.set(f"Scripts list refreshed ({len(scripts)} files).")

    def on_script_selected(self, event=None):
        name = self.scripts_combo_var.get()
        if not name:
            self.current_script_path = None
            self.scripts_text.delete("1.0", tk.END)
            self.status_var.set("No script selected.")
            return

        self.current_script_path = os.path.join(SCRIPTS_DIR, name)
        self.load_current_script()

    def load_current_script(self):
        if not self.current_script_path:
            self.status_var.set("No script selected to load.")
            return

        text = read_whole_file(self.current_script_path)
        self.scripts_text.delete("1.0", tk.END)
        self.scripts_text.insert("1.0", text)
        self.status_var.set(f"Loaded script {self.current_script_path!r}.")

    def save_current_script(self):
        if not self.current_script_path:
            messagebox.showwarning("Scripts", "No script selected to save.")
            return

        text = self.scripts_text.get("1.0", tk.END)
        try:
            write_whole_file(self.current_script_path, text)
            self.status_var.set(f"Saved script {self.current_script_path!r}.")
        except Exception as e:
            messagebox.showerror("Scripts", f"Failed to save script:\n{e}")


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
    def _get_selected_io_path(self):
        """Return the file path for the currently selected I/O row, or None."""
        sel = self.io_tree.selection()
        if not sel:
            return None

        item_id = sel[0]
        values = self.io_tree.item(item_id, "values")
        # columns are ("mode", "desc", "path", "value")
        if len(values) < 3:
            return None

        path = values[2]
        if path in ("", "(?)"):
            return None
        return path

    def bump_selected_io(self, delta: float):
        """
        Increment or decrement the numeric value for the selected I/O's file.

        delta = +1 for "++" button, -1 for "--".
        """
        path = self._get_selected_io_path()
        if not path:
            messagebox.showwarning(
                "I/O",
                "No I/O row selected, or selected entry has no associated file.",
            )
            return

        raw = read_first_line(path)
        # Try to interpret as int or float; default to 0 if nonsense.
        is_int = False
        try:
            if raw == "":
                val = 0
                is_int = True
            elif "." in raw or "e" in raw.lower():
                val = float(raw)
            else:
                val = int(raw)
                is_int = True
        except Exception:
            val = 0
            is_int = True

        new_val = val + delta
        if is_int:
            text = f"{int(new_val)}\n"
        else:
            text = f"{new_val}\n"

        try:
            write_whole_file(path, text)
            self.status_var.set(f"Set I/O file {path!r} to {text.strip()!r}.")
            self.refresh_io()
        except Exception as e:
            messagebox.showerror(
                "I/O",
                f"Failed to write new value to {path!r}:\n{e}",
            )


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

        # When a row is selected, populate the edit form
        self.io_tree.bind("<<TreeviewSelect>>", self.on_io_row_selected)

        btn_frame = ttk.Frame(self.io_frame)
        btn_frame.pack(fill=tk.X, padx=5, pady=5)

        ttk.Button(btn_frame, text="Refresh",
                   command=self.refresh_io).pack(side=tk.LEFT)

        # Adjust the selected I/O value by -1 / +1
        ttk.Button(btn_frame, text="--",
                   command=lambda: self.bump_selected_io(-1)).pack(side=tk.LEFT, padx=5)

        ttk.Button(btn_frame, text="++",
                   command=lambda: self.bump_selected_io(+1)).pack(side=tk.LEFT, padx=5)

        # Write "1" into Update_Flag.ssv so the engine can react
        ttk.Button(btn_frame, text="Trigger Update",
                   command=self.trigger_update_flag).pack(side=tk.LEFT, padx=15)

        io_auto_chk = ttk.Checkbutton(
            btn_frame,
            text="Auto-refresh",
            variable=self.io_auto_var,
            command=self.on_io_auto_toggle,
        )
        io_auto_chk.pack(side=tk.LEFT, padx=10)


        # --- Detail/edit form for a selected I/O entry ---------------------
        form = ttk.LabelFrame(self.io_frame, text="Selected I/O")
        form.pack(fill=tk.X, padx=5, pady=(0, 5))

        row = 0
        ttk.Label(form, text="Mode:").grid(row=row, column=0, sticky="e", padx=5, pady=2)
        ttk.Label(form, textvariable=self.io_selected_mode_var).grid(
            row=row, column=1, sticky="w", padx=5, pady=2
        )

        row += 1
        ttk.Label(form, text="Description:").grid(
            row=row, column=0, sticky="e", padx=5, pady=2
        )
        ttk.Label(form, textvariable=self.io_selected_desc_var, width=60).grid(
            row=row, column=1, columnspan=3, sticky="w", padx=5, pady=2
        )

        row += 1
        ttk.Label(form, text="File:").grid(row=row, column=0, sticky="e", padx=5, pady=2)
        ttk.Label(form, textvariable=self.io_selected_file_var, width=60).grid(
            row=row, column=1, columnspan=3, sticky="w", padx=5, pady=2
        )

        row += 1
        ttk.Label(form, text="Value:").grid(row=row, column=0, sticky="e", padx=5, pady=2)
        self.io_value_entry = ttk.Entry(form, textvariable=self.io_value_var, width=30)
        self.io_value_entry.grid(row=row, column=1, sticky="w", padx=5, pady=2)

        ttk.Button(form, text="Reload value", command=self.reload_io_value).grid(
            row=row, column=2, sticky="w", padx=5, pady=2
        )
        ttk.Button(form, text="Save value", command=self.save_io_value).grid(
            row=row, column=3, sticky="w", padx=5, pady=2
        )

        for c in range(4):
            form.columnconfigure(c, weight=0)


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
        # Remember which path is currently selected (if any)
        try:
            selected_path = self._get_selected_io_path()
        except AttributeError:
            # If _get_selected_io_path doesn't exist yet for some reason,
            # just fall back to no remembered selection.
            selected_path = None

        self.io_tree.delete(*self.io_tree.get_children())
        entries = self._parse_pinout()

        selected_item = None

        for ent in entries:
            val = ""
            if ent["path"] not in ("", "(?)"):
                val = read_first_line(ent["path"])

            item_id = self.io_tree.insert(
                "",
                tk.END,
                values=(ent["mode"], ent["desc"], ent["path"], val),
            )

            # If this row has the same path as the previously selected row,
            # remember its new item id so we can reselect it.
            if selected_path and ent["path"] == selected_path:
                selected_item = item_id

        # Reselect the same row (by path) if we found it again
        if selected_item is not None:
            self.io_tree.selection_set(selected_item)
            self.io_tree.focus(selected_item)
            self.io_tree.see(selected_item)
            # Update the details form so the value field stays in sync
            if hasattr(self, "on_io_row_selected"):
                self.on_io_row_selected()

        self.status_var.set(
            f"Loaded {len(entries)} I/O entries from {PINOUT_FILE!r}."
        )

    def on_io_auto_toggle(self):
        if self.io_auto_var.get():
            self._schedule_io_refresh()

    def _schedule_io_refresh(self):
        if not self.io_auto_var.get():
            return
        self.refresh_io()
        self.after(1000, self._schedule_io_refresh)


    def on_io_row_selected(self, event=None):
        """When the user selects a row in the I/O tree, populate the edit form."""
        sel = self.io_tree.selection()
        if not sel:
            self.io_selected_path = None
            self.io_selected_mode_var.set("")
            self.io_selected_desc_var.set("")
            self.io_selected_file_var.set("")
            self.io_value_var.set("")
            return

        item_id = sel[0]
        values = self.io_tree.item(item_id, "values")
        if len(values) != 4:
            return

        mode, desc, path, value = values
        self.io_selected_path = path if path not in ("", "(?)") else None

        self.io_selected_mode_var.set(mode)
        self.io_selected_desc_var.set(desc)
        self.io_selected_file_var.set(path)
        self.io_value_var.set(value)

    def reload_io_value(self):
        """Re-read the current value from the backing file for the selected I/O."""
        if not self.io_selected_path:
            messagebox.showwarning("I/O", "Selected entry has no file to reload.")
            return

        val = read_first_line(self.io_selected_path)
        self.io_value_var.set(val)
        self.status_var.set(
            f"Reloaded I/O value from {self.io_selected_path!r}."
        )

    def save_io_value(self):
        """Write the edited value back to the backing file (first line)."""
        if not self.io_selected_path:
            messagebox.showwarning("I/O", "Selected entry has no file to save to.")
            return

        val = self.io_value_var.get()
        try:
            # Replace the file with a single line containing the new value
            write_whole_file(self.io_selected_path, str(val) + "\n")
            self.status_var.set(
                f"Saved I/O value to {self.io_selected_path!r}."
            )
            # Refresh the tree so the row shows the new value
            self.refresh_io()
        except Exception as e:
            messagebox.showerror("I/O", f"Failed to save I/O value:\n{e}")


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
        
    def trigger_update_flag(self):
        """Write '1' into Update_Flag.ssv so the engine can pick it up."""
        if touch_flag(UPDATE_FLAG_FILE, "1\n"):
            self.status_var.set(f"Set update flag {UPDATE_FLAG_FILE!r}.")

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

    # --- System Readout Panel tab ------------------------------------------

    def _reload_panel_defs_if_changed(self):
        """Reload Skelly_Panels.ssv if it changed on disk, and update dropdowns."""
        try:
            mtime = os.path.getmtime(SKELLY_PANELS_FILE) if os.path.exists(SKELLY_PANELS_FILE) else None
        except Exception:
            mtime = None

        if mtime == self._panels_mtime:
            return False

        self._panels_mtime = mtime
        self.panel_defs = load_panel_defs(SKELLY_PANELS_FILE)
        self.panel_defs_by_label = {p.label: p for p in self.panel_defs}
        self.panel_defs_by_id = {p.panel_id: p for p in self.panel_defs}

        self.underhood_panel_defs = [p for p in self.panel_defs if p.kind in self._plot_kinds]
        self.underhood_defs_by_label = {p.label: p for p in self.underhood_panel_defs}

        self.readout_panel_defs = [p for p in self.panel_defs if p.kind == "readout"]
        self.readout_defs_by_label = {p.label: p for p in self.readout_panel_defs}

        # Update Under-The-Hood dropdown (if already built)
        if hasattr(self, "underhood_combo"):
            labels = [p.label for p in self.underhood_panel_defs]
            self.underhood_combo.config(values=labels)
            if labels and self.underhood_panel_var.get() not in labels:
                self.underhood_panel_var.set(labels[0])
            if not labels:
                self.underhood_panel_var.set("")

        # Update Readout dropdown (if already built)
        if hasattr(self, "readout_combo"):
            labels = [p.label for p in self.readout_panel_defs]
            self.readout_combo.config(values=labels)
            if labels and self.readout_panel_var.get() not in labels:
                self.readout_panel_var.set(labels[0])
            if not labels:
                self.readout_panel_var.set("")

        return True

    def _build_readout_tab(self):
        top = ttk.Frame(self.readout_frame)
        top.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)

        ctrl = ttk.Frame(top)
        ctrl.pack(fill=tk.X, pady=(0, 5))

        ttk.Label(ctrl, text="Readout:").pack(side=tk.LEFT)

        self.readout_combo = ttk.Combobox(
            ctrl,
            textvariable=self.readout_panel_var,
            state="readonly",
            values=[p.label for p in self.readout_panel_defs] if self.readout_panel_defs else [],
            width=30,
        )
        self.readout_combo.pack(side=tk.LEFT, padx=5)
        self.readout_combo.bind("<<ComboboxSelected>>", lambda e: self.refresh_readout())

        ttk.Button(ctrl, text="Refresh", command=self.refresh_readout).pack(side=tk.LEFT, padx=5)

        auto_chk = ttk.Checkbutton(
            ctrl,
            text="Auto-refresh",
            variable=self.readout_auto_var,
            command=self.on_readout_auto_toggle,
        )
        auto_chk.pack(side=tk.LEFT, padx=10)

        self.readout_notice = ttk.Label(top, text="", foreground="darkred")
        self.readout_notice.pack(anchor="w", pady=(0, 5))

        body = ttk.Frame(top)
        body.pack(fill=tk.BOTH, expand=True)

        # Left: 3 trend plots
        left = ttk.Frame(body)
        left.grid(row=0, column=0, sticky="nsew", padx=(0, 10))

        self.readout_plot_labels = []
        self.readout_trends = []
        for i in range(3):
            row = ttk.Frame(left)
            row.pack(fill=tk.X, pady=4)
            lbl = ttk.Label(row, text=f"Channel {i}")
            lbl.pack(side=tk.LEFT, padx=(0, 8))
            c = MiniTrendCanvas(row, width=380, height=80)
            c.pack(side=tk.LEFT, fill=tk.X, expand=True)
            self.readout_plot_labels.append(lbl)
            self.readout_trends.append(c)

        # Middle: goals + gauges + low/high indicators
        mid = ttk.Frame(body)
        mid.grid(row=0, column=1, sticky="nsew", padx=(0, 10))

        self.readout_goal_lamps = []
        self.readout_goal_gauges = []
        self.readout_low_lamps = []
        self.readout_high_lamps = []

        for i in range(3):
            r = ttk.Frame(mid)
            r.pack(fill=tk.X, pady=6)

            ttk.Label(r, text="Goal").pack(side=tk.LEFT, padx=(0, 4))
            lamp = LampCanvas(r)
            lamp.pack(side=tk.LEFT, padx=(0, 8))
            gauge = VerticalGaugeCanvas(r, width=26, height=70)
            gauge.pack(side=tk.LEFT, padx=(0, 16))

            ttk.Label(r, text="Low").pack(side=tk.LEFT, padx=(0, 4))
            low = LampCanvas(r)
            low.pack(side=tk.LEFT, padx=(0, 12))

            ttk.Label(r, text="High").pack(side=tk.LEFT, padx=(0, 4))
            high = LampCanvas(r)
            high.pack(side=tk.LEFT)

            self.readout_goal_lamps.append(lamp)
            self.readout_goal_gauges.append(gauge)
            self.readout_low_lamps.append(low)
            self.readout_high_lamps.append(high)

        # Right: outputs
        right = ttk.Frame(body)
        right.grid(row=0, column=2, sticky="nsew")

        self.readout_output_labels = []
        self.readout_output_gauges = []
        for i in range(3):
            r = ttk.Frame(right)
            r.pack(fill=tk.X, pady=6)
            lbl = ttk.Label(r, text="Output: --", width=14)
            lbl.pack(side=tk.LEFT, padx=(0, 8))
            gauge = VerticalGaugeCanvas(r, width=26, height=70)
            gauge.pack(side=tk.LEFT)
            self.readout_output_labels.append(lbl)
            self.readout_output_gauges.append(gauge)

        body.rowconfigure(0, weight=1)
        body.columnconfigure(0, weight=2)
        body.columnconfigure(1, weight=1)
        body.columnconfigure(2, weight=1)

        # Setpoint + update controls
        edit = ttk.LabelFrame(top, text="Setpoint / Update")
        edit.pack(fill=tk.X, pady=(10, 0))

        ttk.Label(edit, text="Channel (row):").pack(side=tk.LEFT, padx=(8, 4))
        ch_spin = ttk.Spinbox(edit, from_=0, to=99, textvariable=self.readout_channel_var, width=4, command=self._on_readout_channel_change)
        ch_spin.pack(side=tk.LEFT, padx=(0, 12))

        ttk.Label(edit, text="Step:").pack(side=tk.LEFT)
        step_spin = ttk.Spinbox(edit, from_=-1e9, to=1e9, increment=0.1, textvariable=self.readout_step_var, width=8)
        step_spin.pack(side=tk.LEFT, padx=(4, 12))

        ttk.Button(edit, text="−", width=3, command=self.readout_dec).pack(side=tk.LEFT, padx=(0, 2))
        ttk.Button(edit, text="+", width=3, command=self.readout_inc).pack(side=tk.LEFT, padx=(0, 8))

        ttk.Label(edit, text="Value:").pack(side=tk.LEFT)
        self.readout_setpoint_entry = ttk.Entry(edit, textvariable=self.readout_setpoint_var, width=12)
        self.readout_setpoint_entry.pack(side=tk.LEFT, padx=(4, 8))

        ttk.Button(edit, text="Set", command=self.readout_set_goal).pack(side=tk.LEFT, padx=(0, 10))
        ttk.Button(edit, text="Trigger update + refresh", command=self.readout_trigger_update_and_refresh).pack(side=tk.LEFT)

        self.readout_goal_path_label = ttk.Label(edit, text="", foreground="#555555")
        self.readout_goal_path_label.pack(side=tk.LEFT, padx=(12, 0))

        # Initial paint
        self.refresh_readout()

    def _on_readout_channel_change(self):
        """When channel changes, preload the current goal value into the setpoint box (when possible)."""
        try:
            idx = int(self.readout_channel_var.get())
        except Exception:
            return
        if self._readout_goal_matrix is None:
            return
        if idx < 0 or idx >= len(self._readout_goal_matrix):
            return
        row = self._readout_goal_matrix[idx] if self._readout_goal_matrix else []
        if not row:
            return
        # Only overwrite user input if the entry isn't focused
        if self.focus_get() is self.readout_setpoint_entry:
            return
        try:
            self.readout_setpoint_var.set(float(row[-1]))
        except Exception:
            pass

    def _last_non_none(self, seq):
        for v in reversed(seq or []):
            if v is not None:
                return v
        return None

    def _extract_limits(self, limit_matrix, rows=3):
        """
        Heuristic: accept either:
        - 2*rows lines: low0, high0, low1, high1, ...
        - rows lines with at least 2 values: [low, high, ...]
        Returns list[(low, high)] of length rows (items may be (None,None)).
        """
        out = [(None, None) for _ in range(rows)]
        if not limit_matrix:
            return out
        if len(limit_matrix) >= 2 * rows:
            for i in range(rows):
                lo = self._last_non_none(limit_matrix[2 * i])
                hi = self._last_non_none(limit_matrix[2 * i + 1])
                out[i] = (lo, hi)
            return out
        if len(limit_matrix) >= rows:
            for i in range(rows):
                row = limit_matrix[i]
                if row and len(row) >= 2:
                    out[i] = (row[0], row[1])
            return out
        return out

    def refresh_readout(self):
        self._reload_panel_defs_if_changed()

        if not self.readout_panel_defs:
            self.readout_notice.config(
                text=(
                    "No readout panels defined. Add lines to Skelly_Panels.ssv like:\n"
                    "  my_readout ; Climate Readout ; readout ; temp_hist,heater_hist,cooler_hist,goals,limits,outputs"
                )
            )
            return

        self.readout_notice.config(text="")

        label = self.readout_panel_var.get()
        readout_def = self.readout_defs_by_label.get(label)
        if readout_def is None:
            readout_def = self.readout_panel_defs[0]
            self.readout_panel_var.set(readout_def.label)

        panel_ids = list(readout_def.files or [])
        if len(panel_ids) < 3:
            self.readout_notice.config(text=f"Readout definition {readout_def.label!r} needs at least 3 panel_ids.")
            return

        hist_ids = panel_ids[0:3]
        goal_id = panel_ids[3] if len(panel_ids) >= 4 else None
        limits_id = panel_ids[4] if len(panel_ids) >= 5 else None
        outputs_id = panel_ids[5] if len(panel_ids) >= 6 else None

        # Compute history series (left plots)
        history_series = []
        current_vals = []
        for i, pid in enumerate(hist_ids):
            pdef = self.panel_defs_by_id.get(pid)
            if pdef is None:
                self.readout_plot_labels[i].config(text=f"{pid} (missing)")
                self.readout_trends[i].plot([])
                history_series.append([])
                current_vals.append(None)
                continue

            self.readout_plot_labels[i].config(text=pdef.label)
            data = self._compute_panel_data(pdef)
            if not data or not data.get("series"):
                self.readout_trends[i].plot([])
                history_series.append([])
                current_vals.append(None)
                continue

            s0 = data["series"][0] if data["series"] else []
            # keep a manageable window for the small plot
            window = 120
            s0 = s0[-window:] if len(s0) > window else s0
            self.readout_trends[i].plot(s0)
            history_series.append(s0)
            current_vals.append(self._last_non_none(s0))

        # Pull limits (optional)
        limits = [(None, None)] * 3
        if limits_id:
            ldef = self.panel_defs_by_id.get(limits_id)
            if ldef and ldef.kind == "matrix_rows" and ldef.files:
                limit_mat = load_ssv_matrix(ldef.files[0])
                limits = self._extract_limits(limit_mat, rows=3)

        # Pull goals (optional; also enables setpoint editing if matrix_rows)
        goal_vals = [None, None, None]
        self._readout_goal_path = None
        self._readout_goal_matrix = None
        self._readout_goal_row_count = 0
        if goal_id:
            gdef = self.panel_defs_by_id.get(goal_id)
            if gdef and gdef.kind == "matrix_rows" and gdef.files:
                gpath = gdef.files[0]
                gmat = load_ssv_matrix(gpath)
                self._readout_goal_path = gpath
                self._readout_goal_matrix = gmat
                self._readout_goal_row_count = len(gmat)
                for i in range(min(3, len(gmat))):
                    goal_vals[i] = self._last_non_none(gmat[i])

        # Pull outputs (optional)
        out_vals = [None, None, None]
        if outputs_id:
            odef = self.panel_defs_by_id.get(outputs_id)
            if odef and odef.kind == "matrix_rows" and odef.files:
                omat = load_ssv_matrix(odef.files[0])
                for i in range(min(3, len(omat))):
                    out_vals[i] = self._last_non_none(omat[i])
            elif odef:
                data = self._compute_panel_data(odef)
                if data and data.get("series"):
                    for i in range(min(3, len(data["series"]))):
                        out_vals[i] = self._last_non_none(data["series"][i])

        # Update goal file path label
        if self._readout_goal_path:
            self.readout_goal_path_label.config(text=f"(goal file: {self._readout_goal_path})")
        else:
            self.readout_goal_path_label.config(text="(no writable goal matrix in this readout)")

        # Update indicators
        for i in range(3):
            cur = current_vals[i]
            goal = goal_vals[i]
            lo, hi = limits[i]

            # Goal lamp: green if current is close to goal (heuristic tolerance)
            goal_ok = False
            if cur is not None and goal is not None:
                tol = None
                if lo is not None and hi is not None and hi != lo:
                    tol = abs(hi - lo) * 0.05
                if tol is None:
                    tol = max(0.01, abs(goal) * 0.01)
                goal_ok = abs(cur - goal) <= tol

            self.readout_goal_lamps[i].set(goal_ok)

            # Goal gauge: normalize goal into 0..1 using limits if available, else using local history min/max
            if goal is None:
                self.readout_goal_gauges[i].set(0.0)
            else:
                if lo is not None and hi is not None and hi != lo:
                    self.readout_goal_gauges[i].set((goal - lo) / (hi - lo))
                else:
                    vals = [v for v in history_series[i] if v is not None]
                    if vals:
                        vmin, vmax = min(vals), max(vals)
                        if vmax == vmin:
                            vmax = vmin + 1.0
                        self.readout_goal_gauges[i].set((goal - vmin) / (vmax - vmin))
                    else:
                        self.readout_goal_gauges[i].set(0.0)

            # Low / High indicators (based on current and limits)
            low_on = False
            high_on = False
            if cur is not None and lo is not None:
                low_on = cur < lo
            if cur is not None and hi is not None:
                high_on = cur > hi
            self.readout_low_lamps[i].set(low_on)
            self.readout_high_lamps[i].set(high_on)

            # Output indicator
            ov = out_vals[i]
            on = False
            if ov is not None:
                try:
                    on = float(ov) > 0.5
                except Exception:
                    on = False
            self.readout_output_labels[i].config(text=f"Output: {'ON' if on else 'OFF'}")
            if ov is None:
                self.readout_output_gauges[i].set(0.0)
            else:
                try:
                    fv = float(ov)
                except Exception:
                    fv = 0.0
                # If it's already 0..1, use it; else treat as boolean-ish.
                if 0.0 <= fv <= 1.0:
                    self.readout_output_gauges[i].set(fv)
                else:
                    self.readout_output_gauges[i].set(1.0 if fv > 0 else 0.0)

        # Preload setpoint field for selected channel (when possible)
        self._on_readout_channel_change()

        self.status_var.set(f"Refreshed System Readout Panel {readout_def.label!r}.")

    def on_readout_auto_toggle(self):
        if self.readout_auto_var.get():
            self._schedule_readout_refresh()

    def _schedule_readout_refresh(self):
        if not self.readout_auto_var.get():
            return
        self.refresh_readout()
        self.after(1000, self._schedule_readout_refresh)

    def readout_inc(self):
        try:
            v = float(self.readout_setpoint_var.get())
            step = float(self.readout_step_var.get())
        except Exception:
            v, step = 0.0, 1.0
        self.readout_setpoint_var.set(v + step)

    def readout_dec(self):
        try:
            v = float(self.readout_setpoint_var.get())
            step = float(self.readout_step_var.get())
        except Exception:
            v, step = 0.0, 1.0
        self.readout_setpoint_var.set(v - step)

    def readout_set_goal(self):
        """Write the setpoint into the goal matrix (when this readout provides one)."""
        if not self._readout_goal_path or self._readout_goal_matrix is None:
            messagebox.showerror("Setpoint Error", "This readout does not provide a writable goal matrix (kind=matrix_rows).")
            return

        try:
            ch = int(self.readout_channel_var.get())
        except Exception:
            messagebox.showerror("Setpoint Error", "Invalid channel (row) index.")
            return

        try:
            val = float(self.readout_setpoint_var.get())
        except Exception:
            messagebox.showerror("Setpoint Error", "Invalid numeric setpoint value.")
            return

        mat = list(self._readout_goal_matrix or [])

        if ch < 0:
            messagebox.showerror("Setpoint Error", "Channel index must be >= 0.")
            return

        while len(mat) <= ch:
            mat.append([])

        if not mat[ch]:
            mat[ch] = [val]
        else:
            mat[ch][-1] = val

        try:
            write_ssv_matrix(self._readout_goal_path, mat)
        except Exception as e:
            messagebox.showerror("Setpoint Error", f"Failed to write {self._readout_goal_path!r}:\n{e}")
            return

        # keep local copy in sync
        self._readout_goal_matrix = mat
        self.status_var.set(f"Set goal row {ch} to {val:g} in {self._readout_goal_path!r}.")
        self.refresh_readout()


    def readout_trigger_update_and_refresh(self):
        """Trigger engine update, then refresh a few times to catch the new state."""
        self.trigger_update_flag()
        self._readout_burst_token = getattr(self, "_readout_burst_token", 0) + 1
        self._schedule_readout_burst_refresh(token=self._readout_burst_token, remaining_ms=4000)


    def _schedule_readout_burst_refresh(self, token: int, remaining_ms: int):
        if token != getattr(self, "_readout_burst_token", None):
            return
        self.refresh_readout()
        if remaining_ms <= 0:
            return
        self.after(500, lambda: self._schedule_readout_burst_refresh(token, remaining_ms - 500))

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
            values=[p.label for p in self.underhood_panel_defs] if self.underhood_panel_defs else [],
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
        self._reload_panel_defs_if_changed()

        if not self.underhood_panel_defs:
            self.underhood_canvas.plot(
                [], [],
                message="No Under-The-Hood panels defined (Skelly_Panels.ssv missing/empty, or only non-plottable kinds)."
            )
            self.status_var.set("No Under-The-Hood panels defined.")
            return

        label = self.underhood_panel_var.get()
        panel_def = self.underhood_defs_by_label.get(label)
        if panel_def is None:
            # If the selected label vanished or isn't valid, reset to first
            panel_def = self.underhood_panel_defs[0]
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
        self.refresh_status()
        # Under-The-Hood: only draw once; auto-refresh can take over
        self.refresh_underhood()


if __name__ == "__main__":
    app = GaiaConfigUI()
    app.mainloop()
