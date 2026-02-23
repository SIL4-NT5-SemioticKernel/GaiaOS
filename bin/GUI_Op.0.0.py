#!/usr/bin/env python3
"""
GUI_Op.0.0.py

Gaia Operator Console (Tkinter)

This is an operator-centric redesign of the older "GUI_Skelly" console:
- Left navigation (Operate / Goals / Flow / Outputs / Diagnostics / Advanced)
- Persistent engine header (ticks, update state, quick actions)
- Same file/flag interaction model (pinout.cfg, status.ssv, Update_Flag.ssv, Control_Panel.ssv, etc.)

Notes
- This GUI is intentionally defensive: missing files/directories are handled gracefully.
- Advanced tools (raw SSV editor, scripts, autoexec) are fenced under the Advanced page.

Run:
    python3 GUI_Op.0.0.py
"""

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
UPDATE_FLAG_FILE     = "Update_Flag.ssv"          # engine watches this

STATUS_FILE          = os.path.join(SYSTEM_STATE_DIR, "status.ssv")
ONION_SNAPSHOT_FILE  = os.path.join(SYSTEM_STATE_DIR, "onion.ssv")
SCRIPTS_DIR          = "./scripts"
AUTOEXEC_FILE        = "autoexec.ssv"

# Legacy / common telemetry files (used in Flow & Diagnostics)
DEVIATION_FILE       = os.path.join(SYSTEM_STATE_DIR, "deviation_mapping.ssv")
PROJECTION_FILE      = os.path.join(SYSTEM_STATE_DIR, "projection.ssv")
OUTPUT_SCORES_FILE   = os.path.join(SYSTEM_STATE_DIR, "output_scores.ssv")

TRACE_VALID_FILE         = os.path.join(SYSTEM_STATE_DIR, "trace_valid.ssv")
TRACE_NEARLY_VALID_FILE  = os.path.join(SYSTEM_STATE_DIR, "trace_nearly_valid.ssv")
TRACE_TOTAL_FILE         = os.path.join(SYSTEM_STATE_DIR, "trace_total_output.ssv")
NODE_COUNT_FILE          = os.path.join(SYSTEM_STATE_DIR, "node_count.ssv")
BOREDOM_FILE             = os.path.join(SYSTEM_STATE_DIR, "boredom.ssv")

AFFERENT_LOG_FILE  = os.path.join(SYSTEM_STATE_DIR, "afferent_log.ssv")
EFFERENT_LOG_FILE  = os.path.join(SYSTEM_STATE_DIR, "efferent_log.ssv")

SKELLY_PANELS_FILE = "Skelly_Panels.ssv"   # routing table for Diagnostics plots

# Optional reference diagram (shown on Flow page if present)
FLOW_DIAGRAM_FILE = "2025.GaiaFlow.png"


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
    """Return file contents or '' if missing."""
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

    Expected row format:
        DOMAIN NAME VALUE [INFO ...]
    We only care about the first three tokens.
    """
    engine = {}
    afferents = {}  # idx -> dict
    efferents = {}  # idx -> dict

    rows = parse_status_rows(path)
    for parts in rows:
        if len(parts) < 3:
            continue
        domain, name, val = parts[0], parts[1], parts[2]

        # numeric parse if possible
        try:
            if "." in val or "e" in val.lower():
                v = float(val)
            else:
                v = int(val)
        except Exception:
            v = val

        if domain.upper().startswith("ENG"):
            engine[name] = v
        elif domain.upper().startswith("A"):
            # expected names like A0_Value / A1_Goal / A2_Dev
            # tolerate "A0" domain with "Value"/"Goal"/"Dev" names too
            idx = None
            try:
                digits = "".join(ch for ch in domain if ch.isdigit())
                idx = int(digits) if digits else None
            except Exception:
                idx = None

            # also allow name "A{idx}_Value" style
            if idx is None and name.startswith("A") and "_" in name:
                try:
                    idx = int(name[1:name.index("_")])
                except Exception:
                    idx = None

            if idx is None:
                continue

            d = afferents.setdefault(idx, {})
            low = name.lower()
            if "value" in low:
                d["Value"] = v
            elif "goal" in low:
                d["Goal"] = v
            elif "dev" in low or "deviation" in low:
                d["Dev"] = v
            else:
                d[name] = v

        elif domain.upper().startswith("E"):
            idx = None
            try:
                digits = "".join(ch for ch in domain if ch.isdigit())
                idx = int(digits) if digits else None
            except Exception:
                idx = None

            if idx is None and name.startswith("E") and "_" in name:
                try:
                    idx = int(name[1:name.index("_")])
                except Exception:
                    idx = None

            if idx is None:
                continue

            d = efferents.setdefault(idx, {})
            low = name.lower()
            if "value" in low:
                d["Value"] = v
            else:
                d[name] = v

        else:
            # pass through some common "NT4 Bored X" style markers into engine
            if domain == "NT4" and name == "Bored":
                engine["Bored"] = v

    return engine, afferents, efferents


# ---------------------------------------------------------------------------
# SSV loading helpers
# ---------------------------------------------------------------------------

def load_ssv_matrix(path: str):
    """
    Load whitespace-separated numeric matrix from an SSV file.
    Returns: list[list[float|None]].
    """
    if not os.path.exists(path):
        return []
    mat = []
    try:
        with open(path, "r", encoding="utf-8") as f:
            for raw in f:
                line = raw.strip()
                if not line or line.startswith("#") or line.startswith("//"):
                    continue
                parts = line.split()
                row = []
                for p in parts:
                    try:
                        row.append(float(p))
                    except Exception:
                        row.append(None)
                mat.append(row)
    except Exception:
        return []
    return mat

def load_ssv_xy(path: str):
    """
    Load 2-col x/y numeric series from file.
    Returns: xs, ys
    """
    if not os.path.exists(path):
        return [], []
    xs, ys = [], []
    try:
        with open(path, "r", encoding="utf-8") as f:
            for raw in f:
                line = raw.strip()
                if not line or line.startswith("#") or line.startswith("//"):
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
        series: [[ch0...], [ch1...], ...]
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
# Diagnostics panel definitions
# ---------------------------------------------------------------------------

class PanelDef:
    """
    One line from Skelly_Panels.ssv:

        panel_id ; label ; kind ; files

    - kind: "io_log", "matrix_rows", "xy", "xy_multi"
    """
    def __init__(self, panel_id, label, kind, files):
        self.panel_id = panel_id
        self.label = label
        self.kind = kind
        self.files = files  # list[str]

def load_panel_defs(path: str):
    """
    Load panel definitions from Skelly_Panels.ssv.
    Safe defaults: returns [] if missing / empty / malformed.
    """
    if not os.path.exists(path):
        return []

    defs = []
    try:
        with open(path, "r", encoding="utf-8") as f:
            for raw in f:
                line = raw.strip()
                if not line or line.startswith("#") or line.startswith("//"):
                    continue
                parts = [p.strip() for p in line.split(";")]
                if len(parts) < 4:
                    continue
                panel_id, label, kind, files_str = parts[0], parts[1], parts[2], parts[3]
                files = [s.strip() for s in files_str.split(",") if s.strip()]
                defs.append(PanelDef(panel_id, label, kind, files))
    except Exception:
        return []
    return defs


# ---------------------------------------------------------------------------
# Plotting canvas (lightweight, no external deps)
# ---------------------------------------------------------------------------

class LinePlotCanvas(tk.Canvas):
    def __init__(self, parent, width=800, height=300, **kwargs):
        super().__init__(parent, width=width, height=height, bg="white", **kwargs)
        self.width = width
        self.height = height

    def plot(self, x, series_list, x_label="X", y_label="Y", series_labels=None, message=None):
        self.delete("all")
        w = int(self.winfo_width() or self.width)
        h = int(self.winfo_height() or self.height)

        margin_left = 60
        margin_right = 20
        margin_top = 20
        margin_bottom = 45

        # Empty state
        if message:
            self.create_text(w // 2, h // 2, text=message, fill="gray")
            return
        if not x or not series_list:
            self.create_text(w // 2, h // 2, text="(no data)", fill="gray")
            return

        # Flatten min/max
        y_min = None
        y_max = None
        for series in series_list:
            for v in series:
                if v is None:
                    continue
                y_min = v if y_min is None else min(y_min, v)
                y_max = v if y_max is None else max(y_max, v)

        if y_min is None or y_max is None:
            self.create_text(w // 2, h // 2, text="(no numeric data)", fill="gray")
            return

        if y_min == y_max:
            y_min -= 1
            y_max += 1

        x_min = min(x)
        x_max = max(x)
        if x_min == x_max:
            x_min -= 1
            x_max += 1

        def x_to_px(xv):
            return margin_left + (xv - x_min) / (x_max - x_min) * (w - margin_left - margin_right)

        def y_to_py(yv):
            return margin_top + (y_max - yv) / (y_max - y_min) * (h - margin_top - margin_bottom)

        # Axes
        self.create_line(margin_left, margin_top, margin_left, h - margin_bottom, fill="black")
        self.create_line(margin_left, h - margin_bottom, w - margin_right, h - margin_bottom, fill="black")

        # Labels
        self.create_text(margin_left + 5, margin_top - 10, text=y_label, anchor="w", fill="black")
        self.create_text(w - margin_right, h - margin_bottom + 25, text=x_label, anchor="e", fill="black")

        # Ticks (light)
        for frac in [0.0, 0.5, 1.0]:
            yv = y_min + frac * (y_max - y_min)
            py = y_to_py(yv)
            self.create_line(margin_left - 5, py, margin_left, py, fill="black")
            self.create_text(margin_left - 8, py, text=f"{yv:.2g}", anchor="e", fill="black")

        for frac in [0.0, 0.5, 1.0]:
            xv = x_min + frac * (x_max - x_min)
            px = x_to_px(xv)
            self.create_line(px, h - margin_bottom, px, h - margin_bottom + 5, fill="black")
            self.create_text(px, h - margin_bottom + 16, text=f"{xv:.2g}", anchor="n", fill="black")

        default_colors = ["#1f77b4", "#ff7f0e", "#2ca02c", "#d62728", "#9467bd", "#8c564b"]

        # Lines
        for si, series in enumerate(series_list):
            color = default_colors[si % len(default_colors)]
            pts = []
            for xv, yv in zip(x, series):
                if yv is None:
                    continue
                pts.append((x_to_px(xv), y_to_py(yv)))
            if len(pts) >= 2:
                for (x1, y1), (x2, y2) in zip(pts, pts[1:]):
                    self.create_line(x1, y1, x2, y2, fill=color, width=2)
            elif len(pts) == 1:
                x1, y1 = pts[0]
                self.create_oval(x1-2, y1-2, x1+2, y1+2, fill=color, outline=color)

        # Legend
        if series_labels:
            lx = margin_left + 10
            ly = margin_top + 10
            for si, label in enumerate(series_labels):
                color = default_colors[si % len(default_colors)]
                self.create_rectangle(lx, ly, lx + 10, ly + 10, fill=color, outline=color)
                self.create_text(lx + 14, ly + 5, text=str(label), anchor="w", fill="black")
                ly += 14


# ---------------------------------------------------------------------------
# Reusable editor panels
# ---------------------------------------------------------------------------

class TextFileEditor(ttk.Frame):
    """
    A simple text editor panel for a specific file, optionally with a trigger-flag button.
    """
    def __init__(self, parent, controller, label, filepath, allow_save=True, flag_path=None, flag_label=None):
        super().__init__(parent)
        self.controller = controller
        self.filepath = filepath
        self.allow_save = allow_save
        self.flag_path = flag_path
        self.flag_label_text = flag_label or ""

        ttk.Label(self, text=label).pack(anchor="w", padx=6, pady=(6, 2))

        text_frame = ttk.Frame(self)
        text_frame.pack(fill=tk.BOTH, expand=True, padx=6, pady=6)

        self.text = tk.Text(text_frame, wrap="none", undo=True)
        vsb = ttk.Scrollbar(text_frame, orient="vertical", command=self.text.yview)
        self.text.configure(yscrollcommand=vsb.set)
        self.text.grid(row=0, column=0, sticky="nsew")
        vsb.grid(row=0, column=1, sticky="ns")
        text_frame.rowconfigure(0, weight=1)
        text_frame.columnconfigure(0, weight=1)

        btn = ttk.Frame(self)
        btn.pack(fill=tk.X, padx=6, pady=(0, 6))

        ttk.Button(btn, text="Reload", command=self.reload).pack(side=tk.LEFT)
        if allow_save:
            ttk.Button(btn, text="Save", command=self.save).pack(side=tk.LEFT, padx=6)

        if flag_path:
            ttk.Button(btn, text="Trigger flag", command=self.trigger_flag).pack(side=tk.LEFT, padx=6)
            self.flag_label = ttk.Label(btn, text="")
            self.flag_label.pack(side=tk.LEFT, padx=10)
        else:
            self.flag_label = None

        # Keybinds
        self.text.bind("<Control-s>", lambda e: (self.save(), "break") if self.allow_save else None)
        self.reload()

    def reload(self):
        text = read_whole_file(self.filepath)
        self.text.delete("1.0", tk.END)
        self.text.insert("1.0", text)
        if self.flag_label:
            self._update_flag_label()
        self.controller.set_status(f"Loaded {self.filepath!r}.")

    def save(self):
        if not self.allow_save:
            return
        text = self.text.get("1.0", tk.END)
        try:
            write_whole_file(self.filepath, text)
            self.controller.set_status(f"Saved {self.filepath!r}.")
        except Exception as e:
            messagebox.showerror("Save Error", f"Failed to save {self.filepath}:\n{e}")

    def trigger_flag(self):
        if not self.flag_path:
            return
        if touch_flag(self.flag_path, "1\n"):
            if self.flag_label:
                self._update_flag_label()
            self.controller.set_status(f"Set flag {self.flag_path!r}.")

    def _update_flag_label(self):
        if not self.flag_label or not self.flag_path:
            return
        if os.path.exists(self.flag_path):
            val = read_first_line(self.flag_path)
            self.flag_label.config(text=f"{self.flag_label_text}{val!r}")
        else:
            self.flag_label.config(text=f"{self.flag_label_text}(missing)")


class ScriptsEditor(ttk.Frame):
    def __init__(self, parent, controller):
        super().__init__(parent)
        self.controller = controller
        self.current_script_path = None
        self.scripts_combo_var = tk.StringVar()

        top = ttk.Frame(self)
        top.pack(fill=tk.BOTH, expand=True, padx=6, pady=6)

        hdr = ttk.Frame(top)
        hdr.pack(fill=tk.X, pady=(0, 6))

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
        self.scripts_combo.bind("<<ComboboxSelected>>", lambda e: self.on_script_selected())

        ttk.Button(hdr, text="Rescan", command=self.rescan_scripts).pack(side=tk.LEFT, padx=6)
        ttk.Button(hdr, text="Reload", command=self.load_current_script).pack(side=tk.LEFT, padx=6)
        ttk.Button(hdr, text="Save", command=self.save_current_script).pack(side=tk.LEFT, padx=6)

        text_frame = ttk.Frame(top)
        text_frame.pack(fill=tk.BOTH, expand=True)

        self.text = tk.Text(text_frame, wrap="none", undo=True)
        vsb = ttk.Scrollbar(text_frame, orient="vertical", command=self.text.yview)
        self.text.configure(yscrollcommand=vsb.set)
        self.text.grid(row=0, column=0, sticky="nsew")
        vsb.grid(row=0, column=1, sticky="ns")
        text_frame.rowconfigure(0, weight=1)
        text_frame.columnconfigure(0, weight=1)

        self.text.bind("<Control-s>", lambda e: (self.save_current_script(), "break"))

        # default selection
        scripts = self._get_script_list()
        if scripts:
            self.scripts_combo["values"] = scripts
            self.scripts_combo_var.set(scripts[0])
            self.on_script_selected()

    def _get_script_list(self):
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
            self.scripts_combo_var.set(scripts[0])
            self.on_script_selected()
        self.controller.set_status(f"Scripts list refreshed ({len(scripts)} files).")

    def on_script_selected(self):
        name = self.scripts_combo_var.get()
        if not name:
            self.current_script_path = None
            return
        self.current_script_path = os.path.join(SCRIPTS_DIR, name)
        self.load_current_script()

    def load_current_script(self):
        if not self.current_script_path:
            self.controller.set_status("No script selected to load.")
            return
        text = read_whole_file(self.current_script_path)
        self.text.delete("1.0", tk.END)
        self.text.insert("1.0", text)
        self.controller.set_status(f"Loaded script {self.current_script_path!r}.")

    def save_current_script(self):
        if not self.current_script_path:
            messagebox.showwarning("Scripts", "No script selected to save.")
            return
        text = self.text.get("1.0", tk.END)
        try:
            write_whole_file(self.current_script_path, text)
            self.controller.set_status(f"Saved script {self.current_script_path!r}.")
        except Exception as e:
            messagebox.showerror("Scripts", f"Failed to save script:\n{e}")


class SSVEditor(ttk.Frame):
    """
    Lightweight directory + table editor for .ssv files.
    """
    def __init__(self, parent, controller):
        super().__init__(parent)
        self.controller = controller
        self.current_ssv_dir = SYSTEM_STATE_DIR
        self.ssv_dir_label_var = tk.StringVar(value=f"Directory: {os.path.abspath(self.current_ssv_dir)}")
        self.ssv_combo_var = tk.StringVar()
        self._edit_entry = None
        self._edit_var = None

        top = ttk.Frame(self)
        top.pack(fill=tk.BOTH, expand=True, padx=6, pady=6)

        hdr = ttk.Frame(top)
        hdr.pack(fill=tk.X, pady=3)

        ttk.Label(hdr, textvariable=self.ssv_dir_label_var).pack(side=tk.LEFT, padx=(5, 10))
        ttk.Button(hdr, text="Up", command=self.ssv_go_up_dir).pack(side=tk.LEFT, padx=5)
        ttk.Button(hdr, text="Enter dir", command=self.ssv_enter_dir).pack(side=tk.LEFT, padx=5)

        ttk.Button(hdr, text="Delete ALL .ssv", command=self.delete_all_ssv).pack(side=tk.RIGHT, padx=10)

        mid = ttk.Frame(top)
        mid.pack(fill=tk.X, pady=3)

        ttk.Label(mid, text="Select file:").pack(side=tk.LEFT, padx=(5, 2))
        self.ssv_combo = ttk.Combobox(mid, textvariable=self.ssv_combo_var, state="readonly", width=50)
        self.ssv_combo.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=5)

        ttk.Button(mid, text="Reload list", command=self.refresh_ssv_list).pack(side=tk.LEFT, padx=5)
        ttk.Button(mid, text="Load file", command=self.load_ssv_file).pack(side=tk.LEFT, padx=5)
        ttk.Button(mid, text="Save file", command=self.save_ssv_file).pack(side=tk.LEFT, padx=5)

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

        self.refresh_ssv_list()

    def refresh_ssv_list(self):
        base_dir = self.current_ssv_dir or SYSTEM_STATE_DIR
        base_dir = os.path.abspath(base_dir)
        if not os.path.isdir(base_dir):
            os.makedirs(base_dir, exist_ok=True)

        self.current_ssv_dir = base_dir
        self.ssv_dir_label_var.set(f"Directory: {base_dir}")

        try:
            names = os.listdir(base_dir)
        except Exception:
            names = []

        dirs = [name + "/" for name in names if os.path.isdir(os.path.join(base_dir, name))]
        files = [name for name in names if name.endswith(".ssv") and os.path.isfile(os.path.join(base_dir, name))]

        values = sorted(dirs) + sorted(files)
        self.ssv_combo["values"] = values
        if values:
            cur = self.ssv_combo_var.get()
            self.ssv_combo_var.set(cur if cur in values else values[0])
        else:
            self.ssv_combo_var.set("")

        self.controller.set_status(f"Found {len(files)} .ssv files and {len(dirs)} dirs in {base_dir}/")

    def ssv_go_up_dir(self):
        cur = os.path.abspath(self.current_ssv_dir or SYSTEM_STATE_DIR)
        parent = os.path.dirname(cur)
        self.current_ssv_dir = parent
        self.refresh_ssv_list()

    def ssv_enter_dir(self):
        base_dir = os.path.abspath(self.current_ssv_dir or SYSTEM_STATE_DIR)
        sel = self.ssv_combo_var.get()
        if not sel:
            messagebox.showwarning(".ssv Viewer", "No directory selected.")
            return
        if not sel.endswith("/"):
            messagebox.showwarning(".ssv Viewer", "Selected item is not a directory (doesn't end with '/').")
            return
        dirname = sel.rstrip("/")
        new_path = os.path.join(base_dir, dirname)
        if not os.path.isdir(new_path):
            messagebox.showerror(".ssv Viewer", f"Directory no longer exists:\n{new_path}")
            return
        self.current_ssv_dir = new_path
        self.refresh_ssv_list()

    def load_ssv_file(self):
        fname = self.ssv_combo_var.get()
        if not fname:
            messagebox.showwarning(".ssv Viewer", "No file selected.")
            return
        if fname.endswith("/"):
            messagebox.showwarning(".ssv Viewer", "Selected item is a directory. Use 'Enter dir' instead.")
            return

        base_dir = os.path.abspath(self.current_ssv_dir or SYSTEM_STATE_DIR)
        path = os.path.join(base_dir, fname)
        if not os.path.isfile(path):
            messagebox.showerror(".ssv Viewer", f"File not found: {path}")
            return

        try:
            with open(path, "r", encoding="utf-8") as f:
                raw_lines = [l.strip() for l in f if l.strip() and not l.strip().startswith("#")]
        except Exception as e:
            messagebox.showerror(".ssv Viewer", f"Error reading {path}:\n{e}")
            return

        rows = [line.split() for line in raw_lines]
        if not rows:
            messagebox.showinfo(".ssv Viewer", f"No data rows in {path}.")
            return

        num_cols = max(len(r) for r in rows)
        cols = [f"c{i}" for i in range(num_cols)]
        self.ssv_tree["columns"] = cols
        for c in cols:
            self.ssv_tree.heading(c, text=c)
            self.ssv_tree.column(c, width=110, anchor="w", stretch=True)

        self.ssv_tree.delete(*self.ssv_tree.get_children())
        for r in rows:
            self.ssv_tree.insert("", tk.END, values=r)

        self.controller.set_status(f"Loaded {fname} with {len(rows)} rows.")

    def save_ssv_file(self):
        fname = self.ssv_combo_var.get()
        if not fname:
            messagebox.showwarning(".ssv Viewer", "No file selected.")
            return
        if fname.endswith("/"):
            messagebox.showwarning(".ssv Viewer", "Selected item is a directory.")
            return

        path = os.path.join(self.current_ssv_dir, fname)
        rows = []
        for row_id in self.ssv_tree.get_children():
            values = self.ssv_tree.item(row_id, "values")
            if values:
                rows.append(" ".join(values))
        text = "\n".join(rows) + ("\n" if rows else "")

        try:
            with open(path, "w", encoding="utf-8") as f:
                f.write(text)
        except Exception as e:
            messagebox.showerror(".ssv Viewer", f"Error saving to {path}:\n{e}")
            return

        self.controller.set_status(f"Saved {fname} with {len(rows)} rows.")

    def delete_all_ssv(self):
        base_dir = os.path.abspath(self.current_ssv_dir or SYSTEM_STATE_DIR)
        if not messagebox.askyesno(".ssv Viewer", f"Delete ALL .ssv files in this directory?\n\n{base_dir}"):
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
        self.controller.set_status(f"Deleted {deleted} .ssv files from {base_dir}/")

    def _begin_edit_cell(self, event):
        region = self.ssv_tree.identify("region", event.x, event.y)
        if region != "cell":
            return

        row_id = self.ssv_tree.identify_row(event.y)
        col_id = self.ssv_tree.identify_column(event.x)
        if not row_id or not col_id:
            return

        col_index = int(col_id.replace("#", "")) - 1
        try:
            old_value = self.ssv_tree.item(row_id, "values")[col_index]
        except Exception:
            return

        x, y, width, height = self.ssv_tree.bbox(row_id, col_id)
        self._edit_var = tk.StringVar(value=old_value)
        self._edit_entry = tk.Entry(self.ssv_tree, textvariable=self._edit_var)
        self._edit_entry.place(x=x, y=y, width=width, height=height)
        self._edit_entry.focus()
        self._edit_entry.bind("<Return>", lambda e: self._finish_edit_cell(row_id, col_index))
        self._edit_entry.bind("<Escape>", lambda e: self._cancel_edit_cell())
        self._edit_entry.bind("<FocusOut>", lambda e: self._finish_edit_cell(row_id, col_index))

    def _finish_edit_cell(self, row_id, col_index):
        new_value = self._edit_var.get()
        values = list(self.ssv_tree.item(row_id, "values"))
        if 0 <= col_index < len(values):
            values[col_index] = new_value
            self.ssv_tree.item(row_id, values=values)
        self._cancel_edit_cell()

    def _cancel_edit_cell(self):
        if self._edit_entry:
            self._edit_entry.destroy()
            self._edit_entry = None


# ---------------------------------------------------------------------------
# I/O panel (pinout.cfg)
# ---------------------------------------------------------------------------

class IOStatePanel(ttk.Frame):
    def __init__(self, parent, controller):
        super().__init__(parent)
        self.controller = controller

        self.io_selected_path     = None
        self.io_selected_mode_var = tk.StringVar()
        self.io_selected_desc_var = tk.StringVar()
        self.io_selected_file_var = tk.StringVar()
        self.io_value_var         = tk.StringVar()

        self._build()

    def _parse_pinout(self):
        entries = []
        try:
            with open(PINOUT_FILE, "r", encoding="utf-8") as f:
                for raw_line in f:
                    raw = raw_line.rstrip("\n")
                    line = raw.strip()
                    if not line or line.startswith("//") or line.startswith("#"):
                        continue

                    parts = line.split()
                    if not parts:
                        continue
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

    def _get_selected_io_path(self):
        sel = self.io_tree.selection()
        if not sel:
            return None
        item_id = sel[0]
        values = self.io_tree.item(item_id, "values")
        if len(values) < 3:
            return None
        return values[2]

    def _build(self):
        top = ttk.Frame(self)
        top.pack(fill=tk.BOTH, expand=True, padx=6, pady=6)

        columns = ("mode", "desc", "path", "value")
        self.io_tree = ttk.Treeview(top, columns=columns, show="headings", height=16)

        for col, width in zip(columns, (70, 300, 360, 220)):
            self.io_tree.heading(col, text=col.capitalize())
            self.io_tree.column(col, width=width, anchor="w", stretch=True)

        vsb = ttk.Scrollbar(top, orient="vertical", command=self.io_tree.yview)
        self.io_tree.configure(yscrollcommand=vsb.set)
        self.io_tree.grid(row=0, column=0, sticky="nsew")
        vsb.grid(row=0, column=1, sticky="ns")
        top.rowconfigure(0, weight=1)
        top.columnconfigure(0, weight=1)

        self.io_tree.bind("<<TreeviewSelect>>", self.on_io_row_selected)

        btn_frame = ttk.Frame(self)
        btn_frame.pack(fill=tk.X, padx=6, pady=(0, 6))
        ttk.Button(btn_frame, text="Refresh", command=self.refresh_io).pack(side=tk.LEFT)
        ttk.Button(btn_frame, text="--", command=lambda: self.bump_selected_io(-1)).pack(side=tk.LEFT, padx=6)
        ttk.Button(btn_frame, text="++", command=lambda: self.bump_selected_io(+1)).pack(side=tk.LEFT, padx=6)

        ttk.Button(btn_frame, text="Trigger Update", command=self.controller.trigger_update_flag).pack(side=tk.LEFT, padx=12)

        form = ttk.LabelFrame(self, text="Selected I/O")
        form.pack(fill=tk.X, padx=6, pady=(0, 6))

        row = 0
        ttk.Label(form, text="Mode:").grid(row=row, column=0, sticky="e", padx=5, pady=2)
        ttk.Label(form, textvariable=self.io_selected_mode_var).grid(row=row, column=1, sticky="w", padx=5, pady=2)

        row += 1
        ttk.Label(form, text="Description:").grid(row=row, column=0, sticky="e", padx=5, pady=2)
        ttk.Label(form, textvariable=self.io_selected_desc_var, width=70).grid(row=row, column=1, columnspan=3, sticky="w", padx=5, pady=2)

        row += 1
        ttk.Label(form, text="File:").grid(row=row, column=0, sticky="e", padx=5, pady=2)
        ttk.Label(form, textvariable=self.io_selected_file_var, width=70).grid(row=row, column=1, columnspan=3, sticky="w", padx=5, pady=2)

        row += 1
        ttk.Label(form, text="Value:").grid(row=row, column=0, sticky="e", padx=5, pady=2)
        self.value_entry = ttk.Entry(form, textvariable=self.io_value_var, width=30)
        self.value_entry.grid(row=row, column=1, sticky="w", padx=5, pady=2)

        ttk.Button(form, text="Reload", command=self.reload_io_value).grid(row=row, column=2, padx=5, pady=2)
        ttk.Button(form, text="Write", command=self.save_io_value).grid(row=row, column=3, padx=5, pady=2)

        self.refresh_io()

    def refresh_io(self):
        try:
            selected_path = self._get_selected_io_path()
        except Exception:
            selected_path = None

        self.io_tree.delete(*self.io_tree.get_children())
        entries = self._parse_pinout()
        selected_item = None

        for ent in entries:
            val = ""
            if ent["path"] not in ("", "(?)"):
                val = read_first_line(ent["path"])
            item_id = self.io_tree.insert("", tk.END, values=(ent["mode"], ent["desc"], ent["path"], val))
            if selected_path and ent["path"] == selected_path:
                selected_item = item_id

        if selected_item is not None:
            self.io_tree.selection_set(selected_item)
            self.io_tree.focus(selected_item)
            self.io_tree.see(selected_item)
            self.on_io_row_selected()

        self.controller.set_status(f"Loaded {len(entries)} I/O entries from {PINOUT_FILE!r}.")

    def on_io_row_selected(self, event=None):
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
        if not self.io_selected_path:
            messagebox.showwarning("I/O", "Selected entry has no file to reload.")
            return
        val = read_first_line(self.io_selected_path)
        self.io_value_var.set(val)
        self.controller.set_status(f"Reloaded I/O value from {self.io_selected_path!r}.")

    def save_io_value(self):
        if not self.io_selected_path:
            messagebox.showwarning("I/O", "No selected I/O has a backing file.")
            return
        text = self.io_value_var.get().strip()
        try:
            write_whole_file(self.io_selected_path, text + "\n")
            self.controller.set_status(f"Wrote {text!r} to {self.io_selected_path!r}.")
            self.refresh_io()
        except Exception as e:
            messagebox.showerror("I/O", f"Failed to write {self.io_selected_path!r}:\n{e}")

    def bump_selected_io(self, delta: float):
        path = self._get_selected_io_path()
        if not path:
            messagebox.showwarning("I/O", "No I/O row selected, or selected entry has no associated file.")
            return

        raw = read_first_line(path)
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
        text = f"{int(new_val)}\n" if is_int else f"{new_val}\n"

        try:
            write_whole_file(path, text)
            self.controller.set_status(f"Set I/O file {path!r} to {text.strip()!r}.")
            self.refresh_io()
        except Exception as e:
            messagebox.showerror("I/O", f"Failed to write new value to {path!r}:\n{e}")


# ---------------------------------------------------------------------------
# Diagnostics plot panel (Skelly_Panels.ssv)
# ---------------------------------------------------------------------------

class UnderHoodPanel(ttk.Frame):
    def __init__(self, parent, controller):
        super().__init__(parent)
        self.controller = controller

        self.panel_defs = load_panel_defs(SKELLY_PANELS_FILE)
        self.panel_defs_by_label = {p.label: p for p in self.panel_defs}
        self.panel_var = tk.StringVar(value=self.panel_defs[0].label if self.panel_defs else "")

        self.window_depth_var = tk.IntVar(value=0)  # 0 = all
        self.smooth_window_var = tk.IntVar(value=1) # 1 = off
        self.max_points = 1000

        self._build()

    def _build(self):
        top = ttk.Frame(self)
        top.pack(fill=tk.BOTH, expand=True, padx=6, pady=6)

        ctrl = ttk.Frame(top)
        ctrl.pack(fill=tk.X, pady=(0, 6))

        ttk.Label(ctrl, text="Panel:").pack(side=tk.LEFT)

        self.combo = ttk.Combobox(
            ctrl,
            textvariable=self.panel_var,
            state="readonly",
            values=[p.label for p in self.panel_defs] if self.panel_defs else [],
            width=38,
        )
        self.combo.pack(side=tk.LEFT, padx=6)
        self.combo.bind("<<ComboboxSelected>>", lambda e: self.refresh())

        ttk.Button(ctrl, text="Refresh", command=self.refresh).pack(side=tk.LEFT, padx=6)

        ctrl2 = ttk.Frame(top)
        ctrl2.pack(fill=tk.X, pady=(0, 6))

        ttk.Label(ctrl2, text="Window (last N points, 0 = all):").pack(side=tk.LEFT)
        ttk.Spinbox(ctrl2, from_=0, to=1000000, textvariable=self.window_depth_var, width=8).pack(side=tk.LEFT, padx=6)

        ttk.Label(ctrl2, text="Smoothing (moving avg window, 1 = off):").pack(side=tk.LEFT, padx=(20, 0))
        ttk.Spinbox(ctrl2, from_=1, to=1000, textvariable=self.smooth_window_var, width=6).pack(side=tk.LEFT, padx=6)

        self.canvas = LinePlotCanvas(top)
        self.canvas.pack(fill=tk.BOTH, expand=True)

        if not self.panel_defs:
            self.canvas.plot([], [], message="No panel definitions (Skelly_Panels.ssv missing or empty).")

    def _apply_window_smoothing_downsample(self, x, series_list):
        if not x or not series_list:
            return x, series_list

        n = len(x)
        window = max(0, int(self.window_depth_var.get() or 0))
        smooth_w = max(1, int(self.smooth_window_var.get() or 1))

        if window > 0 and n > window:
            x = x[-window:]
            series_list = [s[-window:] for s in series_list]
            n = len(x)

        if smooth_w > 1:
            def smooth(series):
                out = []
                buf = []
                for v in series:
                    buf.append(v)
                    if len(buf) > smooth_w:
                        buf.pop(0)
                    vals = [b for b in buf if b is not None]
                    out.append(None if not vals else sum(vals) / len(vals))
                return out
            series_list = [smooth(s) for s in series_list]

        max_pts = max(10, int(self.max_points))
        if n > max_pts:
            step = math.ceil(n / max_pts)
            x = x[::step]
            series_list = [s[::step] for s in series_list]

        return x, series_list

    def _compute_panel_data(self, panel_def: PanelDef):
        kind = panel_def.kind
        files = panel_def.files

        if kind == "io_log":
            if not files:
                return None
            ticks, series = load_io_log(files[0])
            labels = [f"ch{i}" for i in range(len(series))]
            return {"x": ticks, "series": series, "x_label": "Tick", "y_label": "Value", "labels": labels}

        if kind == "matrix_rows":
            if not files:
                return None
            mat = load_ssv_matrix(files[0])
            if not mat:
                return None
            n_cols = max(len(row) for row in mat)
            x = list(range(n_cols))
            series = mat
            labels = [f"Row {i}" for i in range(len(series))]
            return {"x": x, "series": series, "x_label": "Index", "y_label": "Value", "labels": labels}

        if kind == "xy":
            if not files:
                return None
            xs, ys = load_ssv_xy(files[0])
            label = os.path.basename(files[0])
            return {"x": xs, "series": [ys], "x_label": "X", "y_label": "Y", "labels": [label]}

        if kind == "xy_multi":
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
                elif len(xs) != len(all_x):
                    continue
                series_list.append(ys)
                labels.append(os.path.basename(path))
            if all_x is None or not series_list:
                return None
            return {"x": all_x, "series": series_list, "x_label": "X", "y_label": "Value", "labels": labels}

        return None

    def refresh(self):
        if not self.panel_defs:
            self.canvas.plot([], [], message="No panel definitions (Skelly_Panels.ssv missing or empty).")
            self.controller.set_status("No diagnostics panels defined.")
            return

        label = self.panel_var.get()
        panel_def = self.panel_defs_by_label.get(label) or self.panel_defs[0]
        self.panel_var.set(panel_def.label)

        data = self._compute_panel_data(panel_def)
        if not data or not data.get("x") or not data.get("series"):
            self.canvas.plot([], [], message=f"No data for panel: {panel_def.label}")
            self.controller.set_status(f"No data for diagnostics panel {panel_def.label!r}.")
            return

        x, series = data["x"], data["series"]
        x, series = self._apply_window_smoothing_downsample(x, series)

        self.canvas.plot(
            x, series,
            x_label=data.get("x_label", "X"),
            y_label=data.get("y_label", "Value"),
            series_labels=data.get("labels"),
        )
        self.controller.set_status(f"Refreshed diagnostics panel {panel_def.label!r}.")


# ---------------------------------------------------------------------------
# Operator pages
# ---------------------------------------------------------------------------

class OperatePage(ttk.Frame):
    def __init__(self, parent, controller):
        super().__init__(parent)
        self.controller = controller
        self.dev_threshold_var = tk.DoubleVar(value=0.0)

        self._build()

    def _build(self):
        outer = ttk.Frame(self)
        outer.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)

        # Top: quick snapshot
        snap = ttk.LabelFrame(outer, text="Now")
        snap.pack(fill=tk.X)

        self.engine_label = ttk.Label(snap, text="Engine: (unknown)", font=("TkDefaultFont", 11, "bold"))
        self.engine_label.pack(anchor="w", padx=8, pady=(6, 2))
        self.tick_label = ttk.Label(snap, text="Ticks: -")
        self.tick_label.pack(anchor="w", padx=8)
        self.mode_label = ttk.Label(snap, text="Update mode: -")
        self.mode_label.pack(anchor="w", padx=8, pady=(0, 6))

        # Middle: three columns
        mid = ttk.Frame(outer)
        mid.pack(fill=tk.BOTH, expand=True, pady=10)

        # Deviations
        left = ttk.LabelFrame(mid, text="Top Deviations (Afferents)")
        left.grid(row=0, column=0, sticky="nsew", padx=(0, 6))

        thr = ttk.Frame(left)
        thr.pack(fill=tk.X, padx=6, pady=(6, 0))
        ttk.Label(thr, text="Show |dev| ≥").pack(side=tk.LEFT)
        ttk.Entry(thr, textvariable=self.dev_threshold_var, width=8).pack(side=tk.LEFT, padx=6)
        ttk.Button(thr, text="Refresh", command=self.refresh).pack(side=tk.RIGHT)

        self.dev_tree = ttk.Treeview(left, columns=("id", "value", "goal", "dev"), show="headings", height=12)
        for col, title, w in zip(("id","value","goal","dev"), ("A","Value","Goal","Dev"), (60,100,100,100)):
            self.dev_tree.heading(col, text=title)
            self.dev_tree.column(col, width=w, anchor="w", stretch=True)
        dev_vsb = ttk.Scrollbar(left, orient="vertical", command=self.dev_tree.yview)
        self.dev_tree.configure(yscrollcommand=dev_vsb.set)
        self.dev_tree.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=(6,0), pady=6)
        dev_vsb.pack(side=tk.LEFT, fill=tk.Y, pady=6)

        # Trajectory quick look (logs)
        center = ttk.LabelFrame(mid, text="Trajectory (recent)")
        center.grid(row=0, column=1, sticky="nsew", padx=6)

        self.traj_canvas = LinePlotCanvas(center, height=260)
        self.traj_canvas.pack(fill=tk.BOTH, expand=True, padx=6, pady=6)

        # Outputs
        right = ttk.LabelFrame(mid, text="Efferent Outputs (status)")
        right.grid(row=0, column=2, sticky="nsew", padx=(6, 0))

        self.out_tree = ttk.Treeview(right, columns=("id","value"), show="headings", height=12)
        for col, title, w in zip(("id","value"), ("E","Value"), (60,120)):
            self.out_tree.heading(col, text=title)
            self.out_tree.column(col, width=w, anchor="w", stretch=True)
        out_vsb = ttk.Scrollbar(right, orient="vertical", command=self.out_tree.yview)
        self.out_tree.configure(yscrollcommand=out_vsb.set)
        self.out_tree.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=(6,0), pady=6)
        out_vsb.pack(side=tk.LEFT, fill=tk.Y, pady=6)

        # Bottom: actions
        actions = ttk.Frame(outer)
        actions.pack(fill=tk.X, pady=(0, 6))
        ttk.Button(actions, text="Trigger Update", command=self.controller.trigger_update_flag).pack(side=tk.LEFT)
        ttk.Button(actions, text="Open Outputs", command=lambda: self.controller.show_page("Outputs")).pack(side=tk.LEFT, padx=6)
        ttk.Button(actions, text="Open Flow", command=lambda: self.controller.show_page("Flow")).pack(side=tk.LEFT, padx=6)
        ttk.Button(actions, text="Open Control Panel", command=lambda: self.controller.show_page("Advanced", tab="Control Panel")).pack(side=tk.LEFT, padx=6)

        mid.rowconfigure(0, weight=1)
        mid.columnconfigure(0, weight=1)
        mid.columnconfigure(1, weight=1)
        mid.columnconfigure(2, weight=1)

    def refresh(self):
        engine, afferents, efferents = parse_status_struct(STATUS_FILE)

        # engine snapshot
        session_tick = engine.get("Session_Tick")
        proc_tick = engine.get("Processor_Tick")
        run_update = engine.get("Run_Update")

        state_text = self.controller.compute_engine_state_text(session_tick, run_update)
        self.engine_label.config(text=state_text)
        self.tick_label.config(text=f"Session tick: {session_tick if session_tick is not None else '-'} | Processor tick: {proc_tick if proc_tick is not None else '-'}")
        self.mode_label.config(text=f"Updates: {'ON' if run_update else 'OFF'}" if run_update is not None else "Updates: -")

        # deviations
        thr = float(self.dev_threshold_var.get() or 0.0)
        items = []
        for idx, d in afferents.items():
            dev = d.get("Dev", None)
            try:
                dev_abs = abs(float(dev))
            except Exception:
                dev_abs = None
            if dev_abs is None:
                continue
            if dev_abs < thr:
                continue
            items.append((dev_abs, idx, d))
        items.sort(reverse=True)

        self.dev_tree.delete(*self.dev_tree.get_children())
        for _, idx, d in items[:60]:
            self.dev_tree.insert("", tk.END, values=(f"A{idx}", d.get("Value",""), d.get("Goal",""), d.get("Dev","")))

        # outputs
        self.out_tree.delete(*self.out_tree.get_children())
        for idx in sorted(efferents.keys()):
            self.out_tree.insert("", tk.END, values=(f"E{idx}", efferents[idx].get("Value","")))

        # trajectory plot: show first few channels of afferent log (if available)
        ticks, series = load_io_log(AFFERENT_LOG_FILE)
        if ticks and series:
            # plot first 3 channels to keep it readable
            s = series[:3]
            labels = [f"A_ch{i}" for i in range(len(s))]
            ticks2, s2 = self.controller.apply_window_downsample(ticks, s, window=300)
            self.traj_canvas.plot(ticks2, s2, x_label="Tick", y_label="Value", series_labels=labels)
        else:
            self.traj_canvas.plot([], [], message="No afferent_log data.")

        self.controller.set_status("Operate view refreshed.")


class GoalsPage(ttk.Frame):
    def __init__(self, parent, controller):
        super().__init__(parent)
        self.controller = controller
        self._build()

    def _build(self):
        outer = ttk.Frame(self)
        outer.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)

        hdr = ttk.Frame(outer)
        hdr.pack(fill=tk.X, pady=(0, 8))
        ttk.Label(hdr, text="Afferent Goals & State", font=("TkDefaultFont", 11, "bold")).pack(side=tk.LEFT)
        ttk.Button(hdr, text="Refresh", command=self.refresh).pack(side=tk.RIGHT)

        cols = ("id", "value", "goal", "dev")
        self.tree = ttk.Treeview(outer, columns=cols, show="headings", height=18)
        for col, width, title in zip(cols, (70, 120, 120, 120), ("Afferent", "Value", "Goal", "Deviation")):
            self.tree.heading(col, text=title)
            self.tree.column(col, width=width, anchor="w", stretch=True)

        vsb = ttk.Scrollbar(outer, orient="vertical", command=self.tree.yview)
        self.tree.configure(yscrollcommand=vsb.set)
        self.tree.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        vsb.pack(side=tk.LEFT, fill=tk.Y)

        side = ttk.Frame(outer)
        side.pack(side=tk.LEFT, fill=tk.Y, padx=(10, 0))

        ttk.Label(side, text="Goal editing is done via Control_Panel.ssv", wraplength=240).pack(anchor="w", pady=(0, 10))
        ttk.Button(side, text="Open Control Panel", command=lambda: self.controller.show_page("Advanced", tab="Control Panel")).pack(fill=tk.X)

        ttk.Separator(side, orient="horizontal").pack(fill=tk.X, pady=10)

        ttk.Button(side, text="Trigger Control Flag", command=self.controller.trigger_control_flag).pack(fill=tk.X)
        ttk.Button(side, text="Trigger Update", command=self.controller.trigger_update_flag).pack(fill=tk.X, pady=(6,0))

    def refresh(self):
        _, afferents, _ = parse_status_struct(STATUS_FILE)
        self.tree.delete(*self.tree.get_children())
        for idx in sorted(afferents.keys()):
            d = afferents[idx]
            self.tree.insert("", tk.END, values=(f"A{idx}", d.get("Value",""), d.get("Goal",""), d.get("Dev","")))
        self.controller.set_status("Goals view refreshed.")


class FlowPage(ttk.Frame):
    """
    Pipeline-centric inspector: stage buttons + plot + raw table.
    """
    def __init__(self, parent, controller):
        super().__init__(parent)
        self.controller = controller

        self.stage_var = tk.StringVar(value="Current State")
        self._diagram_img = None

        self.stages = [
            ("Current State",      ("io_log", [AFFERENT_LOG_FILE]), STATUS_FILE),
            ("Symbolic Projection",("xy",     [PROJECTION_FILE]),    PROJECTION_FILE),
            ("Deviation Mapping",  ("matrix_rows",[DEVIATION_FILE]), DEVIATION_FILE),
            ("Reverse Inference",  ("matrix_rows",[OUTPUT_SCORES_FILE]), OUTPUT_SCORES_FILE),
            ("Trace Validity",     ("xy_multi",[TRACE_VALID_FILE, TRACE_NEARLY_VALID_FILE, TRACE_TOTAL_FILE]), TRACE_TOTAL_FILE),
            ("Node Count",         ("xy", [NODE_COUNT_FILE]), NODE_COUNT_FILE),
            ("Corrective Output",  ("io_log", [EFFERENT_LOG_FILE]), EFFERENT_LOG_FILE),
        ]

        self._build()

    def _build(self):
        outer = ttk.Frame(self)
        outer.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)

        top = ttk.Frame(outer)
        top.pack(fill=tk.X, pady=(0, 8))

        ttk.Label(top, text="Flow Inspector", font=("TkDefaultFont", 11, "bold")).pack(side=tk.LEFT)
        ttk.Button(top, text="Refresh", command=self.refresh).pack(side=tk.RIGHT)

        # Stage buttons
        bar = ttk.Frame(outer)
        bar.pack(fill=tk.X, pady=(0, 8))

        for name, _, _ in self.stages:
            ttk.Radiobutton(bar, text=name, value=name, variable=self.stage_var, command=self.refresh).pack(side=tk.LEFT, padx=4)

        # Split: plot + table (with optional diagram)
        pw = ttk.PanedWindow(outer, orient=tk.VERTICAL)
        pw.pack(fill=tk.BOTH, expand=True)

        upper = ttk.Frame(pw)
        lower = ttk.Frame(pw)
        pw.add(upper, weight=3)
        pw.add(lower, weight=2)

        # Upper: plot + diagram on right (if present)
        upper_grid = ttk.Frame(upper)
        upper_grid.pack(fill=tk.BOTH, expand=True)

        self.plot = LinePlotCanvas(upper_grid, height=300)
        self.plot.grid(row=0, column=0, sticky="nsew", padx=(0, 8))

        self.diagram_label = ttk.Label(upper_grid)
        self.diagram_label.grid(row=0, column=1, sticky="nsew")

        upper_grid.rowconfigure(0, weight=1)
        upper_grid.columnconfigure(0, weight=3)
        upper_grid.columnconfigure(1, weight=2)

        # try to load diagram
        self._load_diagram()

        # Lower: raw table view of the stage's main file
        table_frame = ttk.LabelFrame(lower, text="Raw View")
        table_frame.pack(fill=tk.BOTH, expand=True)

        self.raw_tree = ttk.Treeview(table_frame, show="headings")
        vsb = ttk.Scrollbar(table_frame, orient="vertical", command=self.raw_tree.yview)
        self.raw_tree.configure(yscrollcommand=vsb.set)
        self.raw_tree.grid(row=0, column=0, sticky="nsew", padx=(6,0), pady=6)
        vsb.grid(row=0, column=1, sticky="ns", pady=6)

        table_frame.rowconfigure(0, weight=1)
        table_frame.columnconfigure(0, weight=1)

        self.refresh()

    def _load_diagram(self):
        # Load optional diagram for quick grounding
        for candidate in (FLOW_DIAGRAM_FILE, os.path.join(SYSTEM_STATE_DIR, FLOW_DIAGRAM_FILE)):
            if os.path.exists(candidate):
                try:
                    # PhotoImage supports PNG/GIF (PNG OK on most Tk builds)
                    self._diagram_img = tk.PhotoImage(file=candidate)
                    # scale down if huge
                    w = self._diagram_img.width()
                    h = self._diagram_img.height()
                    max_w = 380
                    if w > max_w:
                        scale = max(1, int(w / max_w))
                        self._diagram_img = self._diagram_img.subsample(scale, scale)
                    self.diagram_label.configure(image=self._diagram_img)
                    self.diagram_label.configure(text="")
                    return
                except Exception:
                    pass
        self.diagram_label.configure(text="(flow diagram not found)")
        self.diagram_label.configure(image="")
        self._diagram_img = None

    def _compute_by_kind(self, kind, files):
        if kind == "io_log":
            ticks, series = load_io_log(files[0]) if files else ([], [])
            labels = [f"ch{i}" for i in range(len(series))]
            return ticks, series, "Tick", "Value", labels
        if kind == "matrix_rows":
            mat = load_ssv_matrix(files[0]) if files else []
            if not mat:
                return [], [], "Index", "Value", []
            n_cols = max(len(r) for r in mat)
            x = list(range(n_cols))
            labels = [f"Row {i}" for i in range(len(mat))]
            return x, mat, "Index", "Value", labels
        if kind == "xy":
            xs, ys = load_ssv_xy(files[0]) if files else ([], [])
            label = os.path.basename(files[0]) if files else "series"
            return xs, [ys], "X", "Y", [label]
        if kind == "xy_multi":
            all_x = None
            series_list = []
            labels = []
            for p in (files or []):
                xs, ys = load_ssv_xy(p)
                if not xs or not ys:
                    continue
                if all_x is None:
                    all_x = xs
                elif len(xs) != len(all_x):
                    continue
                series_list.append(ys)
                labels.append(os.path.basename(p))
            return (all_x or []), series_list, "X", "Value", labels
        return [], [], "X", "Y", []

    def _load_table(self, filepath):
        rows = parse_status_rows(filepath) if filepath.endswith("status.ssv") else []
        if not rows:
            # generic whitespace rows
            try:
                with open(filepath, "r", encoding="utf-8") as f:
                    rows = []
                    for raw in f:
                        line = raw.strip()
                        if not line or line.startswith("#") or line.startswith("//"):
                            continue
                        rows.append(line.split())
            except Exception:
                rows = []

        if rows:
            num_cols = max(len(r) for r in rows)
        else:
            num_cols = 0

        columns = [f"c{i}" for i in range(num_cols)]
        self.raw_tree["columns"] = columns
        for idx, col in enumerate(columns):
            self.raw_tree.heading(col, text=f"Col {idx}")
            self.raw_tree.column(col, width=110, anchor="w", stretch=True)

        self.raw_tree.delete(*self.raw_tree.get_children())
        for r in rows:
            padded = list(r) + [""] * (num_cols - len(r))
            self.raw_tree.insert("", tk.END, values=padded)

    def refresh(self):
        name = self.stage_var.get()
        stage = next((s for s in self.stages if s[0] == name), None)
        if not stage:
            self.plot.plot([], [], message="Unknown stage.")
            return

        _, (kind, files), raw_file = stage

        x, series, xlab, ylab, labels = self._compute_by_kind(kind, files)
        x, series = self.controller.apply_window_downsample(x, series, window=500)

        if x and series:
            self.plot.plot(x, series, x_label=xlab, y_label=ylab, series_labels=labels)
        else:
            self.plot.plot([], [], message=f"No data for stage: {name}")

        if raw_file and os.path.exists(raw_file):
            self._load_table(raw_file)
        else:
            self.raw_tree.delete(*self.raw_tree.get_children())
            self.raw_tree["columns"] = []
        self.controller.set_status(f"Flow stage refreshed: {name}")


class OutputsPage(ttk.Frame):
    def __init__(self, parent, controller):
        super().__init__(parent)
        self.controller = controller
        self._build()

    def _build(self):
        outer = ttk.Frame(self)
        outer.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)

        hdr = ttk.Frame(outer)
        hdr.pack(fill=tk.X, pady=(0, 8))
        ttk.Label(hdr, text="Outputs / I/O", font=("TkDefaultFont", 11, "bold")).pack(side=tk.LEFT)
        ttk.Button(hdr, text="Trigger Update", command=self.controller.trigger_update_flag).pack(side=tk.RIGHT)

        self.io_panel = IOStatePanel(outer, self.controller)
        self.io_panel.pack(fill=tk.BOTH, expand=True)

    def refresh(self):
        self.io_panel.refresh_io()


class DiagnosticsPage(ttk.Frame):
    def __init__(self, parent, controller):
        super().__init__(parent)
        self.controller = controller
        self._build()

    def _build(self):
        outer = ttk.Frame(self)
        outer.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)

        hdr = ttk.Frame(outer)
        hdr.pack(fill=tk.X, pady=(0, 8))
        ttk.Label(hdr, text="Diagnostics", font=("TkDefaultFont", 11, "bold")).pack(side=tk.LEFT)

        pw = ttk.PanedWindow(outer, orient=tk.VERTICAL)
        pw.pack(fill=tk.BOTH, expand=True)

        top = ttk.Frame(pw)
        bottom = ttk.Frame(pw)
        pw.add(top, weight=3)
        pw.add(bottom, weight=2)

        self.underhood = UnderHoodPanel(top, self.controller)
        self.underhood.pack(fill=tk.BOTH, expand=True)

        # raw status table
        table = ttk.LabelFrame(bottom, text="status.ssv (raw)")
        table.pack(fill=tk.BOTH, expand=True, padx=6, pady=6)

        self.status_tree = ttk.Treeview(table, show="headings", height=8)
        vsb = ttk.Scrollbar(table, orient="vertical", command=self.status_tree.yview)
        self.status_tree.configure(yscrollcommand=vsb.set)
        self.status_tree.grid(row=0, column=0, sticky="nsew", padx=(6,0), pady=6)
        vsb.grid(row=0, column=1, sticky="ns", pady=6)

        table.rowconfigure(0, weight=1)
        table.columnconfigure(0, weight=1)

        btn = ttk.Frame(table)
        btn.grid(row=1, column=0, sticky="ew", padx=6, pady=(0, 6))
        ttk.Button(btn, text="Refresh status", command=self.refresh_status).pack(side=tk.LEFT)

        self.refresh_status()

    def refresh_status(self):
        rows = parse_status_rows(STATUS_FILE)
        num_cols = max((len(r) for r in rows), default=0)
        columns = [f"c{i}" for i in range(num_cols)]
        self.status_tree["columns"] = columns
        for idx, col in enumerate(columns):
            self.status_tree.heading(col, text=f"Col {idx}")
            self.status_tree.column(col, width=110, anchor="w", stretch=True)
        self.status_tree.delete(*self.status_tree.get_children())
        for r in rows:
            padded = list(r) + [""] * (num_cols - len(r))
            self.status_tree.insert("", tk.END, values=padded)
        self.controller.set_status(f"Loaded {len(rows)} status rows from {STATUS_FILE!r}.")

    def refresh(self):
        self.underhood.refresh()
        self.refresh_status()


class AdvancedPage(ttk.Frame):
    def __init__(self, parent, controller):
        super().__init__(parent)
        self.controller = controller
        self.nb = ttk.Notebook(self)
        self.nb.pack(fill=tk.BOTH, expand=True)

        self.panels = {}

        self._build()

    def _build(self):
        # Control panel
        cp = TextFileEditor(
            self.nb, self.controller,
            label=f"Control Panel file: {CONTROL_PANEL_FILE}",
            filepath=CONTROL_PANEL_FILE,
            allow_save=True,
            flag_path=CONTROL_PANEL_FLAG,
            flag_label="Flag: ",
        )
        self.nb.add(cp, text="Control Panel")
        self.panels["Control Panel"] = cp

        # Autoexec
        ae = TextFileEditor(
            self.nb, self.controller,
            label=f"Autoexec script: {AUTOEXEC_FILE}",
            filepath=AUTOEXEC_FILE,
            allow_save=True,
            flag_path=None,
        )
        self.nb.add(ae, text="Autoexec")
        self.panels["Autoexec"] = ae

        # Onion snapshot (view-only)
        onion = TextFileEditor(
            self.nb, self.controller,
            label=f"Onion snapshot file: {ONION_SNAPSHOT_FILE}",
            filepath=ONION_SNAPSHOT_FILE,
            allow_save=False,
            flag_path=None,
        )
        self.nb.add(onion, text="Onion")
        self.panels["Onion"] = onion

        # Scripts
        scripts = ScriptsEditor(self.nb, self.controller)
        self.nb.add(scripts, text="Scripts")
        self.panels["Scripts"] = scripts

        # SSV editor
        ssv = SSVEditor(self.nb, self.controller)
        self.nb.add(ssv, text="SSV Editor")
        self.panels["SSV Editor"] = ssv

    def select_tab(self, tab_name: str):
        panel = self.panels.get(tab_name)
        if not panel:
            return
        idx = list(self.panels.keys()).index(tab_name)
        self.nb.select(idx)

    def refresh(self):
        # refresh current tab if it has reload/refresh
        cur = self.nb.select()
        if not cur:
            return
        widget = self.nametowidget(cur)
        if hasattr(widget, "reload"):
            try:
                widget.reload()
            except Exception:
                pass
        if hasattr(widget, "refresh_ssv_list"):
            try:
                widget.refresh_ssv_list()
            except Exception:
                pass


# ---------------------------------------------------------------------------
# App shell
# ---------------------------------------------------------------------------

class GaiaOperatorApp(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Gaia Operator Console")
        self.geometry("1280x820")

        self.status_var = tk.StringVar(value="Ready.")
        self.auto_refresh_var = tk.BooleanVar(value=False)
        self.refresh_interval_ms = 1000

        self._last_session_tick = None

        self._build_shell()
        self._build_pages()

        self.show_page("Operate")
        self.refresh_all()

        # One loop to rule them all
        self._refresh_loop()

    # controller API used by pages ------------------------------------------------

    def set_status(self, text: str):
        self.status_var.set(text)

    def trigger_update_flag(self):
        if touch_flag(UPDATE_FLAG_FILE, "1\n"):
            self.set_status(f"Set update flag {UPDATE_FLAG_FILE!r}.")

    def trigger_control_flag(self):
        if touch_flag(CONTROL_PANEL_FLAG, "1\n"):
            self.set_status(f"Set control panel flag {CONTROL_PANEL_FLAG!r}.")

    def compute_engine_state_text(self, session_tick, run_update):
        # Use tick deltas to infer "running" vs "idle"
        state_text = "Engine: unknown"
        if session_tick is not None:
            if self._last_session_tick is None:
                state_text = f"Engine: active (tick {session_tick})"
            else:
                try:
                    if session_tick > self._last_session_tick:
                        state_text = "Engine: running"
                    else:
                        state_text = "Engine: no tick change (idle or stopped)"
                except Exception:
                    state_text = f"Engine: tick={session_tick}"
        if session_tick is not None:
            self._last_session_tick = session_tick
        if isinstance(run_update, (int, float)):
            state_text += " | Updates: ON" if run_update else " | Updates: OFF"
        return state_text

    def apply_window_downsample(self, x, series_list, window=500, max_points=1000):
        """
        Shared helper for operator plots:
        - window: keep last N points (if > 0)
        - downsample to <= max_points
        """
        if not x or not series_list:
            return x, series_list

        try:
            window = int(window)
        except Exception:
            window = 0
        if window > 0 and len(x) > window:
            x = x[-window:]
            series_list = [s[-window:] for s in series_list]

        n = len(x)
        if n > max_points:
            step = math.ceil(n / max_points)
            x = x[::step]
            series_list = [s[::step] for s in series_list]
        return x, series_list

    # shell -----------------------------------------------------------------

    def _build_shell(self):
        self.columnconfigure(0, weight=1)
        self.rowconfigure(1, weight=1)

        # Header
        header = ttk.Frame(self, padding=(10, 8))
        header.grid(row=0, column=0, sticky="ew")
        header.columnconfigure(2, weight=1)

        self.h_engine = ttk.Label(header, text="Engine: (unknown)", font=("TkDefaultFont", 11, "bold"))
        self.h_engine.grid(row=0, column=0, sticky="w")

        self.h_ticks = ttk.Label(header, text="Ticks: -")
        self.h_ticks.grid(row=1, column=0, sticky="w", pady=(2,0))

        btns = ttk.Frame(header)
        btns.grid(row=0, column=2, rowspan=2, sticky="e")

        ttk.Button(btns, text="Refresh", command=self.refresh_all).pack(side=tk.LEFT)
        ttk.Button(btns, text="Trigger Update", command=self.trigger_update_flag).pack(side=tk.LEFT, padx=6)

        ttk.Checkbutton(btns, text="Auto-refresh", variable=self.auto_refresh_var).pack(side=tk.LEFT, padx=10)

        # Body: nav + page container
        body = ttk.Frame(self)
        body.grid(row=1, column=0, sticky="nsew")
        body.columnconfigure(1, weight=1)
        body.rowconfigure(0, weight=1)

        nav = ttk.Frame(body, padding=(8, 10))
        nav.grid(row=0, column=0, sticky="nsw")
        nav.rowconfigure(10, weight=1)

        self.nav_buttons = {}
        for i, name in enumerate(["Operate", "Goals", "Flow", "Outputs", "Diagnostics", "Advanced"]):
            b = ttk.Button(nav, text=name, command=lambda n=name: self.show_page(n), width=16)
            b.grid(row=i, column=0, sticky="ew", pady=3)
            self.nav_buttons[name] = b

        ttk.Separator(nav, orient="horizontal").grid(row=7, column=0, sticky="ew", pady=(12, 6))

        ttk.Button(nav, text="Exit", command=self.destroy).grid(row=8, column=0, sticky="ew", pady=3)

        self.page_container = ttk.Frame(body)
        self.page_container.grid(row=0, column=1, sticky="nsew")
        self.page_container.rowconfigure(0, weight=1)
        self.page_container.columnconfigure(0, weight=1)

        # Status bar
        status = ttk.Label(self, textvariable=self.status_var, anchor="w")
        status.grid(row=2, column=0, sticky="ew")

    def _build_pages(self):
        self.pages = {}

        self.pages["Operate"] = OperatePage(self.page_container, self)
        self.pages["Goals"] = GoalsPage(self.page_container, self)
        self.pages["Flow"] = FlowPage(self.page_container, self)
        self.pages["Outputs"] = OutputsPage(self.page_container, self)
        self.pages["Diagnostics"] = DiagnosticsPage(self.page_container, self)
        self.pages["Advanced"] = AdvancedPage(self.page_container, self)

        for p in self.pages.values():
            p.grid(row=0, column=0, sticky="nsew")

    def show_page(self, name: str, tab: str = None):
        page = self.pages.get(name)
        if not page:
            return
        page.tkraise()

        # nav highlight (simple)
        for n, b in self.nav_buttons.items():
            b.state(["pressed"] if n == name else ["!pressed"])

        if name == "Advanced" and tab and hasattr(page, "select_tab"):
            try:
                page.select_tab(tab)
            except Exception:
                pass

        self.current_page = name
        self.set_status(f"Showing page: {name}")

        # Refresh on navigation for responsiveness
        try:
            if hasattr(page, "refresh"):
                page.refresh()
        except Exception:
            pass
        self.refresh_header()

    def refresh_header(self):
        engine, _, _ = parse_status_struct(STATUS_FILE)
        session_tick = engine.get("Session_Tick")
        proc_tick = engine.get("Processor_Tick")
        run_update = engine.get("Run_Update")
        bored = engine.get("Bored", None)

        self.h_engine.config(text=self.compute_engine_state_text(session_tick, run_update))
        self.h_ticks.config(text=f"Session tick: {session_tick if session_tick is not None else '-'} | Processor tick: {proc_tick if proc_tick is not None else '-'}"
                                 + (f" | Bored: {bored}" if bored is not None else ""))

    def refresh_all(self):
        self.refresh_header()
        # refresh current page only (keeps the UI responsive)
        if self.current_page:
            page = self.pages.get(self.current_page)
            if page and hasattr(page, "refresh"):
                try:
                    page.refresh()
                except Exception:
                    pass

    def _refresh_loop(self):
        if self.auto_refresh_var.get():
            self.refresh_all()
        self.after(self.refresh_interval_ms, self._refresh_loop)


if __name__ == "__main__":
    app = GaiaOperatorApp()
    app.mainloop()
