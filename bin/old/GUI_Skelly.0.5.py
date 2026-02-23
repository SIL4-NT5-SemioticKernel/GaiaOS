#!/usr/bin/env python3
import os
import math
import re
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
# Small dashboard widgets (Status / Goals)
# ---------------------------------------------------------------------------

class LedIndicator(tk.Canvas):
    """Tiny circular LED indicator."""

    def __init__(self, master, size=14, on_color="green", off_color="#9a9a9a", **kwargs):
        bg = kwargs.pop("bg", None)
        super().__init__(master, width=size, height=size, highlightthickness=0, bg=bg, **kwargs)
        self._on_color = on_color
        self._off_color = off_color
        pad = 2
        self._oval = self.create_oval(
            pad, pad, size - pad, size - pad,
            outline="#222222", width=1, fill=self._off_color
        )

    def set_on(self, on: bool, on_color=None, off_color=None):
        if on_color is not None:
            self._on_color = on_color
        if off_color is not None:
            self._off_color = off_color
        self.itemconfigure(self._oval, fill=(self._on_color if on else self._off_color))


class MiniPlotCanvas(tk.Canvas):
    """Lightweight grid + line plot for per-channel history."""

    def __init__(self, master, width=420, height=80, **kwargs):
        bg = kwargs.pop("bg", "#f5f5f5")
        super().__init__(
            master, width=width, height=height,
            bg=bg, highlightthickness=1, highlightbackground="#2b2b2b", **kwargs
        )

    def plot(self, x_data, y_data, title=None, goal=None, x_hint=None, message=None):
        """Draw a small scrolling history plot.

        - Oldest samples appear on the left, newest on the right.
        - If `goal` is numeric, draws a horizontal goal line.
        - Adds a compact title and y-scale labels for readability.
        """
        self.delete("all")
        self.update_idletasks()

        w = max(int(self.winfo_width()), 10)
        h = max(int(self.winfo_height()), 10)

        # Layout margins (keep very small, this is a tiny plot)
        m_left = 44
        m_right = 6
        m_top = 16
        m_bottom = 14

        plot_w = max(w - m_left - m_right, 2)
        plot_h = max(h - m_top - m_bottom, 2)
        x0 = m_left
        y0 = m_top
        x1 = m_left + plot_w
        y1 = m_top + plot_h

        def _fmt(v):
            try:
                v = float(v)
            except Exception:
                return str(v)
            av = abs(v)
            if av >= 1000:
                return f"{v:.0f}"
            if av >= 100:
                return f"{v:.1f}"
            if av >= 10:
                return f"{v:.2f}"
            if av >= 1:
                return f"{v:.3f}"
            return f"{v:.4f}"

        # Title
        if title:
            self.create_text(2, 2, text=str(title), fill="#111111", anchor="nw")

        if message is not None:
            self.create_text(w // 2, h // 2, text=message, fill="#333333")
            return

        if not x_data or not y_data:
            self.create_text(w // 2, h // 2, text="(no data)", fill="#333333")
            return

        # compute bounds ignoring None
        xs = [x for x in x_data if isinstance(x, (int, float))]
        ys = [y for y in y_data if isinstance(y, (int, float))]
        if not xs or not ys:
            self.create_text(w // 2, h // 2, text="(no numeric data)", fill="#333333")
            return

        x_min, x_max = min(xs), max(xs)
        y_min, y_max = min(ys), max(ys)
        if isinstance(goal, (int, float)):
            y_min = min(y_min, float(goal))
            y_max = max(y_max, float(goal))

        if x_max == x_min:
            x_max = x_min + 1.0
        if y_max == y_min:
            y_max = y_min + 1.0

        # padding
        y_pad = 0.06 * (y_max - y_min)
        y_min -= y_pad
        y_max += y_pad

        # Background grid (inside plot area only)
        grid_px = 10
        for gx in range(x0, x1 + 1, grid_px):
            self.create_line(gx, y0, gx, y1, fill="#d0d0d0")
        for gy in range(y0, y1 + 1, grid_px):
            self.create_line(x0, gy, x1, gy, fill="#d0d0d0")

        # Axes
        self.create_line(x0, y0, x0, y1, fill="#2b2b2b")      # y-axis
        self.create_line(x0, y1, x1, y1, fill="#2b2b2b")      # x-axis

        # y-scale labels
        self.create_text(x0 - 4, y0, text=_fmt(y_max), fill="#111111", anchor="ne")
        self.create_text(x0 - 4, y1, text=_fmt(y_min), fill="#111111", anchor="se")

        # x hint (optional)
        if x_hint:
            self.create_text(x1, y1 + 2, text=str(x_hint), fill="#111111", anchor="nw")

        def x_to_px(x):
            return x0 + (x - x_min) / (x_max - x_min) * plot_w

        def y_to_px(y):
            return y1 - (y - y_min) / (y_max - y_min) * plot_h

        # goal line
        if isinstance(goal, (int, float)):
            gy = y_to_px(float(goal))
            self.create_line(x0, gy, x1, gy, fill="#1b5cb8", dash=(3, 3))
            self.create_text(x1 - 2, gy - 2, text=f"goal {_fmt(goal)}", fill="#1b5cb8", anchor="se")

        # draw polyline segments (break on None)
        pts = []
        last_xy = None
        for x, y in zip(x_data, y_data):
            if not isinstance(x, (int, float)) or not isinstance(y, (int, float)):
                if len(pts) >= 4:
                    self.create_line(
                        *pts,
                        fill="#d11f1f",
                        width=3,
                        capstyle="round",
                        joinstyle="round"
                    )
                pts = []
                last_xy = None
                continue
            px = x_to_px(float(x))
            py = y_to_px(float(y))
            pts.extend([px, py])
            last_xy = (px, py)

        if len(pts) >= 4:
            self.create_line(
                *pts,
                fill="#d11f1f",
                width=3,
                capstyle="round",
                joinstyle="round"
            )

        # marker at the newest point (helps visually confirm updates on flat signals)
        if last_xy is not None:
            px, py = last_xy
            r = 2
            self.create_oval(px - r, py - r, px + r, py + r, fill="#d11f1f", outline="")


class _ScrollableFrame(ttk.Frame):
    """A ttk.Frame with a vertical scrollbar."""

    def __init__(self, master, **kwargs):
        super().__init__(master, **kwargs)
        self._canvas = tk.Canvas(self, highlightthickness=0)
        self._vsb = ttk.Scrollbar(self, orient="vertical", command=self._canvas.yview)
        self._canvas.configure(yscrollcommand=self._vsb.set)

        self._inner = ttk.Frame(self._canvas)
        self._inner_id = self._canvas.create_window((0, 0), window=self._inner, anchor="nw")

        self._canvas.grid(row=0, column=0, sticky="nsew")
        self._vsb.grid(row=0, column=1, sticky="ns")
        self.rowconfigure(0, weight=1)
        self.columnconfigure(0, weight=1)

        self._inner.bind("<Configure>", self._on_inner_configure)
        self._canvas.bind("<Configure>", self._on_canvas_configure)

        # mouse wheel support (best-effort)
        self._canvas.bind_all("<MouseWheel>", self._on_mousewheel)

    @property
    def inner(self):
        return self._inner

    def _on_inner_configure(self, _event=None):
        self._canvas.configure(scrollregion=self._canvas.bbox("all"))

    def _on_canvas_configure(self, event):
        # keep inner width in sync with canvas width
        self._canvas.itemconfigure(self._inner_id, width=event.width)

    def _on_mousewheel(self, event):
        try:
            # Windows / Mac (delta sign differs but division works)
            self._canvas.yview_scroll(int(-1 * (event.delta / 120)), "units")
        except Exception:
            pass

# ---------------------------------------------------------------------------
# Main UI
# ---------------------------------------------------------------------------

class GaiaConfigUI(tk.Tk):
    def __init__(self):
        super().__init__()

        self.title("GaiaOS Control / Status Console")
        self.geometry("1200x800")

        # auto-refresh toggles
        self.summary_auto_var    = tk.BooleanVar(value=True)
        self._summary_after_id     = None
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
        
        # panel defs for Under-The-Hood
        self.panel_defs          = load_panel_defs(SKELLY_PANELS_FILE)
        self.panel_defs_by_label = {p.label: p for p in self.panel_defs}
        self.underhood_panel_var = tk.StringVar(value=self.panel_defs[0].label if self.panel_defs else "")
        
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
        # --- Summary dashboard history buffer (in-GUI rolling window) ---
        self.dashboard_history_points_var = tk.IntVar(value=180)  # 0 = unlimited
        self.dashboard_histories = {}  # (kind, idx) -> {"x": deque, "y": deque}
        self._dashboard_row_defs = []  # cached slot defs (kind, idx, label)
        self._dashboard_sample_counter = 0

        self._build_widgets()
        self.refresh_all()


        # start summary auto-refresh loop (for dashboard history)
        if self.summary_auto_var.get():
            self._schedule_summary_refresh()

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

        # Order: put Status/Goals first as the friendly front tab
        self.nb.add(self.summary_frame,   text="Status / Goals")
        self.nb.add(self.io_frame,        text="I/O State")
        self.nb.add(self.ssv_frame,       text="SSV Files")
        self.nb.add(self.scripts_frame,   text="Scripts")       # NEW
        self.nb.add(self.underhood_frame, text="Under The Hood")
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
        outer = ttk.Frame(self.summary_frame)
        outer.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)

        # Inside Status/Goals, default to a dashboard view, but keep the old tables.
        self.summary_nb = ttk.Notebook(outer)
        self.summary_nb.pack(fill=tk.BOTH, expand=True)

        self.summary_dashboard_frame = ttk.Frame(self.summary_nb)
        self.summary_tables_frame = ttk.Frame(self.summary_nb)

        self.summary_nb.add(self.summary_dashboard_frame, text="Dashboard")
        self.summary_nb.add(self.summary_tables_frame, text="Tables")

        self._build_summary_dashboard()
        self._build_summary_tables()

    def _build_summary_dashboard(self):
        top = ttk.Frame(self.summary_dashboard_frame)
        top.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)

        # Header / controls
        hdr = ttk.Frame(top)
        hdr.pack(fill=tk.X, pady=(0, 6))

        self.dash_engine_var = tk.StringVar(value="Engine: unknown")
        ttk.Label(hdr, textvariable=self.dash_engine_var).pack(side=tk.LEFT)

        ttk.Label(hdr, text="History points:").pack(side=tk.LEFT, padx=(15, 3))
        self.dash_points_spin = ttk.Spinbox(
            hdr,
            from_=0,
            to=5000,
            increment=10,
            textvariable=self.dashboard_history_points_var,
            width=7,
        )
        self.dash_points_spin.pack(side=tk.LEFT)

        ttk.Button(hdr, text="Clear history", command=self._dashboard_clear_history).pack(
            side=tk.LEFT, padx=8
        )

        ttk.Button(hdr, text="Refresh", command=self.refresh_summary).pack(side=tk.RIGHT)

        auto_chk = ttk.Checkbutton(
            hdr,
            text="Auto-refresh",
            variable=self.summary_auto_var,
            command=self.on_summary_auto_toggle,
        )
        auto_chk.pack(side=tk.RIGHT, padx=(0, 10))

        # Column headers
        cols = ttk.Frame(top)
        cols.pack(fill=tk.X, pady=(0, 4))
        ttk.Label(cols, text="History").grid(row=0, column=0, sticky="w", padx=(5, 0))
        ttk.Label(cols, text="Goal").grid(row=0, column=1, sticky="w", padx=(10, 0))
        ttk.Label(cols, text="Output").grid(row=0, column=2, sticky="w", padx=(10, 0))
        cols.columnconfigure(0, weight=1)

        # Scrollable rows area
        self.dash_rows_scroller = _ScrollableFrame(top)
        self.dash_rows_scroller.pack(fill=tk.BOTH, expand=True)

        self.dash_rows_container = self.dash_rows_scroller.inner
        self.dash_row_widgets = {}  # (kind, idx) -> widget dict

        # Footer shortcuts (kept from the older summary view)
        ftr = ttk.Frame(top)
        ftr.pack(fill=tk.X, pady=(6, 0))

        ttk.Button(
            ftr, text="Open Control Panel", command=lambda: self.nb.select(self.ctrl_frame)
        ).pack(side=tk.LEFT)

        ttk.Button(
            ftr, text="Open I/O State", command=lambda: self.nb.select(self.io_frame)
        ).pack(side=tk.LEFT, padx=6)

        ttk.Button(
            ftr, text="Open Under The Hood", command=lambda: self.nb.select(self.underhood_frame)
        ).pack(side=tk.LEFT, padx=6)

        ttk.Button(
            ftr, text="Open System Status", command=lambda: self.nb.select(self.status_frame)
        ).pack(side=tk.LEFT, padx=6)

    def _build_summary_tables(self):
        # This preserves the previous "Status / Goals" content.
        top = ttk.Frame(self.summary_tables_frame)
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

    def _dashboard_clear_history(self):
        self.dashboard_histories = {}
        self._dashboard_sample_counter = 0
        # refresh plots immediately
        self._dashboard_refresh_from_status(None, {}, {})


    def _dashboard_get_slots(self, afferents, efferents):
        """Return ordered dashboard slots.

        The intent is: show rows in pinout.cfg order, but *map them onto the
        real A#/E# indices that exist in status.ssv*, so the dashboard doesn't
        go blank when pinout ordering doesn't match engine indexing.

        Any remaining channels present in status.ssv but not referenced by
        pinout.cfg are appended as "(unmapped)" rows.
        """
        try:
            entries = self._parse_pinout()
        except Exception:
            entries = []

        if entries:
            return self._dashboard_map_pinout_to_status_slots(entries, afferents, efferents)

        return self._dashboard_get_slots_from_status(afferents, efferents)

    def _dashboard_extract_index(self, ent, kind):
        """Best-effort extraction of an explicit A#/E# index from a pinout entry."""
        try:
            raw = " ".join([
                str(ent.get("mode", "")),
                str(ent.get("desc", "")),
                str(ent.get("path", "")),
                str(ent.get("raw", "")),
            ])
        except Exception:
            raw = str(ent)

        # strict: A12 / E7
        m = re.search(rf"\b{kind}(\d+)\b", raw, flags=re.IGNORECASE)
        if m:
            try:
                return int(m.group(1))
            except Exception:
                pass

        # loose: Afferent_12, Efferent-3, etc
        m = re.search(rf"\b{kind}[^0-9]{{0,8}}(\d+)\b", raw, flags=re.IGNORECASE)
        if m:
            try:
                return int(m.group(1))
            except Exception:
                pass

        return None

    def _dashboard_map_pinout_to_status_slots(self, entries, afferents, efferents):
        aff_avail = sorted(afferents.keys()) if isinstance(afferents, dict) else []
        eff_avail = sorted(efferents.keys()) if isinstance(efferents, dict) else []
        used_aff = set()
        used_eff = set()

        slots = []

        def _next_unused(avail, used):
            for i in avail:
                if i not in used:
                    return i
            return None

        # Map pinout rows first (keeps the human-facing order)
        for ent in entries:
            mode = (ent.get("mode") or "").strip()
            kind = "E" if mode.startswith("E") else "A"

            if kind == "A":
                idx = self._dashboard_extract_index(ent, "A")
                if idx is None or idx not in aff_avail or idx in used_aff:
                    idx = _next_unused(aff_avail, used_aff)
                if idx is not None:
                    used_aff.add(idx)
                else:
                    # keep the row, but it will be "unmapped" (no engine channel available)
                    idx = -(1000 + sum(1 for k, i, _ in slots if k == "A" and i < 0))
                slots.append(("A", idx, ent))

            else:
                idx = self._dashboard_extract_index(ent, "E")
                if idx is None or idx not in eff_avail or idx in used_eff:
                    idx = _next_unused(eff_avail, used_eff)
                if idx is not None:
                    used_eff.add(idx)
                else:
                    idx = -(2000 + sum(1 for k, i, _ in slots if k == "E" and i < 0))
                slots.append(("E", idx, ent))

        # Append any remaining channels from status.ssv that weren't referenced/mapped.
        for idx in aff_avail:
            if idx not in used_aff:
                slots.append(("A", idx, {"mode": "A", "desc": f"(unmapped) A{idx}", "path": "", "raw": ""}))
        for idx in eff_avail:
            if idx not in used_eff:
                slots.append(("E", idx, {"mode": "E", "desc": f"(unmapped) E{idx}", "path": "", "raw": ""}))

        return slots


    def _dashboard_get_slots_from_status(self, afferents, efferents):
        slots = []
        for idx in sorted(afferents.keys()):
            slots.append(("A", idx, {"mode": "A", "desc": f"A{idx}", "path": ""}))
        for idx in sorted(efferents.keys()):
            slots.append(("E", idx, {"mode": "E", "desc": f"E{idx}", "path": ""}))
        return slots

    def _dashboard_ensure_rows(self, slots):
        """Build row widgets if the slot definitions changed."""
        sig = [(k, i, (ent.get("desc") if isinstance(ent, dict) else str(ent))) for (k, i, ent) in slots]
        if sig == self._dashboard_row_defs and self.dash_row_widgets:
            return

        # clear existing
        for child in list(self.dash_rows_container.winfo_children()):
            child.destroy()
        self.dash_row_widgets = {}
        self._dashboard_row_defs = sig

        for r, (kind, idx, ent) in enumerate(slots):
            desc = ent.get("desc", f"{kind}{idx}") if isinstance(ent, dict) else str(ent)
            idx_lbl = f"{kind}{idx}" if isinstance(idx, int) and idx >= 0 else f"{kind}?"
            label_txt = f"{idx_lbl} — {desc}"

            row = ttk.Frame(self.dash_rows_container, relief="ridge", padding=6)
            row.grid(row=r, column=0, sticky="ew", padx=4, pady=4)
            row.columnconfigure(0, weight=1)

            # left: label + plot
            left = ttk.Frame(row)
            left.grid(row=0, column=0, sticky="nsew")
            left.columnconfigure(1, weight=1)

            name_lbl = ttk.Label(left, text=label_txt)
            name_lbl.grid(row=0, column=0, sticky="w")

            val_var = tk.StringVar(value="")
            val_lbl = ttk.Label(left, textvariable=val_var)
            val_lbl.grid(row=0, column=1, sticky="e", padx=(10, 0))

            plot = MiniPlotCanvas(left, width=520, height=90)
            plot.grid(row=1, column=0, columnspan=2, sticky="ew", pady=(4, 0))

            # middle: goal + low/high
            mid = ttk.Frame(row)
            mid.grid(row=0, column=1, sticky="ns", padx=(12, 0))
            ttk.Label(mid, text="Goal").grid(row=0, column=0, sticky="w")

            goal_led = LedIndicator(mid, size=14, on_color="#15a44a")
            goal_led.grid(row=0, column=1, padx=6)

            goal_val_var = tk.StringVar(value="")
            ttk.Label(mid, textvariable=goal_val_var).grid(row=0, column=2, sticky="w")

            # low/high indicators
            ttk.Label(mid, text="Low").grid(row=1, column=0, sticky="w", pady=(6, 0))
            low_led = LedIndicator(mid, size=14, on_color="#15a44a")
            low_led.grid(row=1, column=1, padx=6, pady=(6, 0))

            ttk.Label(mid, text="High").grid(row=1, column=2, sticky="w", pady=(6, 0))
            high_led = LedIndicator(mid, size=14, on_color="#15a44a")
            high_led.grid(row=1, column=3, padx=6, pady=(6, 0))

            # right: output state
            right = ttk.Frame(row)
            right.grid(row=0, column=2, sticky="ns", padx=(12, 0))
            ttk.Label(right, text="Output").grid(row=0, column=0, sticky="w")

            out_led = LedIndicator(right, size=14, on_color="#d11f1f")
            out_led.grid(row=0, column=1, padx=6)

            out_var = tk.StringVar(value="")
            ttk.Label(right, textvariable=out_var).grid(row=0, column=2, sticky="w")

            self.dash_row_widgets[(kind, idx)] = {
                "label": name_lbl,
                "value_var": val_var,
                "plot": plot,
                "goal_led": goal_led,
                "goal_val_var": goal_val_var,
                "low_led": low_led,
                "high_led": high_led,
                "out_led": out_led,
                "out_var": out_var,
                "ent": ent,
            }

        self.dash_rows_container.columnconfigure(0, weight=1)

    def _dashboard_refresh_from_status(self, engine, afferents, efferents):
        # choose slot source (pinout order mapped onto status indices when possible)
        slots = self._dashboard_get_slots(afferents, efferents)

        self._dashboard_ensure_rows(slots)


        # x-axis sample index (monotonic within this GUI session).
        # This guarantees the history trace "scrolls" even if Session_Tick is static.
        x_now = float(self._dashboard_sample_counter)
        self._dashboard_sample_counter += 1.0

        # window length
        try:
            maxlen = int(self.dashboard_history_points_var.get())
        except Exception:
            maxlen = 180
        if maxlen < 0:
            maxlen = 0

        def _hist_for(key):
            h = self.dashboard_histories.get(key)
            if h is None:
                from collections import deque
                h = {"x": deque(maxlen=(maxlen if maxlen else None)), "y": deque(maxlen=(maxlen if maxlen else None))}
                self.dashboard_histories[key] = h
            else:
                # update maxlen if user changed it
                if maxlen:
                    if h["x"].maxlen != maxlen:
                        from collections import deque
                        h["x"] = deque(h["x"], maxlen=maxlen)
                        h["y"] = deque(h["y"], maxlen=maxlen)
                else:
                    if h["x"].maxlen is not None:
                        from collections import deque
                        h["x"] = deque(h["x"], maxlen=None)
                        h["y"] = deque(h["y"], maxlen=None)
            return h

        for kind, idx, ent in slots:
            w = self.dash_row_widgets.get((kind, idx))
            if not w:
                continue

            val = None
            goal = None

            if kind == "A":
                d = afferents.get(idx, {}) if isinstance(afferents, dict) else {}
                val = d.get("Value")
                goal = d.get("Goal")
            else:
                d = efferents.get(idx, {}) if isinstance(efferents, dict) else {}
                val = d.get("Value")

            # history update (only numeric values)
            key = (kind, idx)
            h = _hist_for(key)
            if isinstance(val, (int, float)):
                h["x"].append(float(x_now))
                h["y"].append(float(val))
            else:
                # still advance x, but break the polyline segment
                h["x"].append(float(x_now))
                h["y"].append(None)

            # value label
            if isinstance(val, (int, float)):
                w["value_var"].set(f"{val:g}")
            else:
                w["value_var"].set("" if val is None else str(val))

            # goal indicator
            has_goal = isinstance(goal, (int, float))
            w["goal_led"].set_on(has_goal)
            w["goal_val_var"].set(f"{goal:g}" if has_goal else "")

            # low/high indicator (only when a numeric goal exists)
            low_on = False
            high_on = False
            if has_goal and isinstance(val, (int, float)):
                if val < goal:
                    low_on = True
                elif val > goal:
                    high_on = True
            w["low_led"].set_on(low_on)
            w["high_led"].set_on(high_on)

            # output indicator (for efferents)
            out_on = False
            if kind == "E" and isinstance(val, (int, float)):
                out_on = bool(val)
            w["out_led"].set_on(out_on)
            if kind == "E":
                w["out_var"].set("ON" if out_on else "OFF")
            else:
                w["out_var"].set("")

            # plot (oldest on left, newest on right)
            x_list = list(h["x"])
            y_list = list(h["y"])

            # title: prefer the human description from pinout, but keep the A#/E# prefix stable
            desc = ""
            if isinstance(ent, dict):
                desc = (ent.get("desc") or "").strip()
            idx_label = f"{kind}{idx}" if isinstance(idx, int) and idx >= 0 else f"{kind}?"
            title = idx_label + (f" — {desc}" if desc else "")

            if not any(isinstance(v, (int, float)) for v in y_list):
                w["plot"].plot([], [], title=title, message="(no numeric data yet)")
            else:
                w["plot"].plot(
                    x_list, y_list,
                    title=title,
                    goal=(goal if has_goal else None),
                    x_hint="oldest → newest"
                )


    def refresh_summary(self):
        engine, afferents, efferents = parse_status_struct(STATUS_FILE)


        # Dashboard (history graphs / goals / outputs)
        if hasattr(self, "dash_row_widgets"):
            try:
                self._dashboard_refresh_from_status(engine, afferents, efferents)
            except Exception as e:
                # Surface errors instead of silently failing (silent failures look like "no updates").
                try:
                    import traceback
                    traceback.print_exc()
                except Exception:
                    pass
                if hasattr(self, "status_var"):
                    self.status_var.set(f"Dashboard update error: {e}")

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
            state_text += " | Updates: ON" if run_update else " | Updates: OFF"

        if isinstance(exit_flag, (int, float)) and exit_flag:
            state_text += " | EXIT requested"

        # dashboard header (if built)
        if hasattr(self, "dash_engine_var"):
            self.dash_engine_var.set(state_text)

        # tables header (if built)
        if hasattr(self, "summary_engine_label"):
            self.summary_engine_label.config(text=state_text)
        if hasattr(self, "summary_tick_label"):
            self.summary_tick_label.config(
                text=f"Session tick: {session_tick if session_tick is not None else '-'}"
            )
        if hasattr(self, "summary_proc_label"):
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
                            boredom_text = "Engagement: BORED" if bored_val >= 0.5 else "Engagement: exploring"
                        break
        except FileNotFoundError:
            boredom_text = "Engagement: (no status file)"

        if hasattr(self, "summary_boredom_label"):
            self.summary_boredom_label.config(text=boredom_text)

        # Update tables (if present)
        if hasattr(self, "summary_aff_tree"):
            self.summary_aff_tree.delete(*self.summary_aff_tree.get_children())
            for idx in sorted(afferents.keys()):
                d = afferents[idx]
                val = d.get("Value", "")
                goal = d.get("Goal", "")
                dev = d.get("Dev", "")
                self.summary_aff_tree.insert("", tk.END, values=(f"A{idx}", val, goal, dev))

        if hasattr(self, "summary_eff_tree"):
            self.summary_eff_tree.delete(*self.summary_eff_tree.get_children())
            for idx in sorted(efferents.keys()):
                d = efferents[idx]
                val = d.get("Value", "")
                self.summary_eff_tree.insert("", tk.END, values=(f"E{idx}", val))

        # Update dashboard rows/plots
        if hasattr(self, "_dashboard_refresh_from_status"):
            self._dashboard_refresh_from_status(engine, afferents, efferents)

        self.status_var.set("Summary / Goals updated.")


    def on_summary_auto_toggle(self):
        # Ensure only one timer is running
        if self.summary_auto_var.get():
            if self._summary_after_id is None:
                self._schedule_summary_refresh()
        else:
            if self._summary_after_id is not None:
                try:
                    self.after_cancel(self._summary_after_id)
                except Exception:
                    pass
                self._summary_after_id = None


    def _schedule_summary_refresh(self):
        if not self.summary_auto_var.get():
            self._summary_after_id = None
            return

        # pull latest status + extend history + redraw dashboard
        self.refresh_summary()

        # reschedule
        self._summary_after_id = self.after(1000, self._schedule_summary_refresh)


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
        self.refresh_status()
        # Under-The-Hood: only draw once; auto-refresh can take over
        self.refresh_underhood()


if __name__ == "__main__":
    app = GaiaConfigUI()
    app.mainloop()
