#!/usr/bin/env python3
"""
GaiaOS File-Bridge GUI (Tkinter)
- No external dependencies (Tkinter is in the standard library)
- Works alongside your RPi_Driver.py + Gaia C++ server

What it does:
- Parses pinout.cfg to discover afferents (A/A1W/US/PH) and efferents (E)
- Displays current sensor values from IO_Files/A/*.ssv
- Lets you set/toggle actuator outputs to IO_Files/E/*.ssv
- Shows Control_Panel flag/state, and can trigger a manual eval tick
"""

import os
import sys
import time
import tempfile
from pathlib import Path
import tkinter as tk
from tkinter import ttk, messagebox, filedialog

# --- Project-relative files (default names) ---
PINOUT_CFG                = Path("pinout.cfg")
IO_A_DIR                  = Path("IO_Files/A")
IO_E_DIR                  = Path("IO_Files/E")
CONTROL_PANEL_FILE        = Path("Control_Panel.ssv")
CONTROL_PANEL_FLAG_FILE   = Path("Control_Panel_Flag.ssv")
CONTROL_PANEL_FINISHED    = Path("Control_Panel_Finished.ssv")

# --- GUI polling interval (ms) ---
DEFAULT_REFRESH_MS = 500  # 0.5s UI update

# ------------------------------------------------------------
# Utilities
# ------------------------------------------------------------
def ensure_dirs():
    IO_A_DIR.mkdir(parents=True, exist_ok=True)
    IO_E_DIR.mkdir(parents=True, exist_ok=True)

def write_atomic(path: Path, data: str):
    """Atomic file write to avoid partial reads by other processes."""
    path.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile("w", dir=str(path.parent), delete=False) as tf:
        tf.write(data)
        tf.flush()
        os.fsync(tf.fileno())
    os.replace(tf.name, str(path))

def safe_read_text(path: Path, default: str = "") -> str:
    try:
        return path.read_text().strip()
    except Exception:
        return default

def parse_pinout(pinout_path: Path):
    """
    Returns (afferents, efferents)
    afferents: list of dicts: {"type": "...", "desc": "...", "path": Path}
    efferents: list of dicts: {"pin": int,   "path": Path}
    """
    aff, eff = [], []
    if not pinout_path.exists():
        return aff, eff

    with pinout_path.open("r") as f:
        for raw in f:
            line = raw.strip()
            if not line or line.startswith("//"):
                continue
            parts = line.split()
            mode = parts[0].upper()

            try:
                if mode == "A":
                    pins = list(map(int, parts[1:-1]))
                    fpath = Path(parts[-1])
                    aff.append({
                        "type": "gpio",
                        "desc": f"GPIO bus {pins}",
                        "path": fpath
                    })
                elif mode == "A1W":
                    sensor_id = parts[1]
                    fpath = Path(parts[2])
                    aff.append({
                        "type": "1wire",
                        "desc": f"1-Wire {sensor_id}",
                        "path": fpath
                    })
                elif mode == "US":
                    trig, echo = map(int, parts[1:3])
                    fpath = Path(parts[3])
                    aff.append({
                        "type": "ultrasonic",
                        "desc": f"HC-SR04 TRIG={trig} ECHO={echo}",
                        "path": fpath
                    })
                elif mode == "PH":
                    addr_hex = parts[1]
                    chan = int(parts[2])
                    fpath = Path(parts[3])
                    aff.append({
                        "type": "ph",
                        "desc": f"pH ADS1115@{addr_hex} CH{chan}",
                        "path": fpath
                    })
                elif mode == "E":
                    pin = int(parts[1])
                    fpath = Path(parts[2])
                    eff.append({
                        "pin": pin,
                        "path": fpath
                    })
            except Exception:
                # Skip malformed lines silently, keep it shop-friendly
                continue
    return aff, eff

# ------------------------------------------------------------
# GUI
# ------------------------------------------------------------
class GaiaGUI(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("GaiaOS Bridge GUI")
        self.geometry("980x620")
        self.minsize(900, 560)

        ensure_dirs()

        # State
        self.refresh_ms = tk.IntVar(value=DEFAULT_REFRESH_MS)
        self.auto_poke = tk.BooleanVar(value=False)  # optional: GUI pokes Gaia if flag != "1"
        self.eval_label = tk.StringVar(value="gui")
        self.eval_threshold = tk.DoubleVar(value=1.1)

        # Data from pinout.cfg
        self.afferents, self.efferents = parse_pinout(PINOUT_CFG)

        # Build UI
        self._build_menu()
        self._build_layout()

        # Start polling loop
        self.after(self.refresh_ms.get(), self._refresh)

    # ---------- UI Construction ----------
    def _build_menu(self):
        m = tk.Menu(self)
        self.config(menu=m)

        file_menu = tk.Menu(m, tearoff=0)
        file_menu.add_command(label="Reload pinout.cfg", command=self.reload_pinout)
        file_menu.add_separator()
        file_menu.add_command(label="Open repo root…", command=self._open_repo_root)
        file_menu.add_separator()
        file_menu.add_command(label="Quit", command=self.destroy)
        m.add_cascade(label="File", menu=file_menu)

        actions_menu = tk.Menu(m, tearoff=0)
        actions_menu.add_command(label="Manual tick (write control script + set flag)", command=self.manual_tick)
        actions_menu.add_command(label="Clear Control_Panel_Flag (set 0)", command=self.clear_flag)
        actions_menu.add_command(label="Mark Finished (write Control_Panel_Finished=1)", command=self.mark_finished)
        m.add_cascade(label="Actions", menu=actions_menu)

        help_menu = tk.Menu(m, tearoff=0)
        help_menu.add_command(label="Where are my files?", command=self._help_paths)
        help_menu.add_command(label="About", command=lambda: messagebox.showinfo("About", "GaiaOS Bridge GUI (Tkinter)"))
        m.add_cascade(label="Help", menu=help_menu)

    def _build_layout(self):
        # Top frame: Control panel status + controls
        top = ttk.LabelFrame(self, text="Control Panel")
        top.pack(fill="x", padx=8, pady=8)

        ttk.Label(top, text="Flag file:").grid(row=0, column=0, sticky="w", padx=6, pady=4)
        self.flag_val = ttk.Label(top, text="(unknown)")
        self.flag_val.grid(row=0, column=1, sticky="w", padx=6, pady=4)

        ttk.Label(top, text="Last Control_Panel.ssv:").grid(row=1, column=0, sticky="w", padx=6, pady=4)
        self.cp_preview = ttk.Label(top, text="—")
        self.cp_preview.grid(row=1, column=1, sticky="w", padx=6, pady=4)

        ttk.Label(top, text="Eval label:").grid(row=0, column=2, sticky="e", padx=6)
        ttk.Entry(top, textvariable=self.eval_label, width=16).grid(row=0, column=3, sticky="w", padx=6)

        ttk.Label(top, text="Threshold:").grid(row=0, column=4, sticky="e", padx=6)
        ttk.Entry(top, textvariable=self.eval_threshold, width=8).grid(row=0, column=5, sticky="w", padx=6)

        ttk.Button(top, text="Manual tick", command=self.manual_tick).grid(row=0, column=6, padx=6)
        ttk.Button(top, text="Flag=0", command=self.clear_flag).grid(row=0, column=7, padx=6)
        ttk.Button(top, text="Finished=1", command=self.mark_finished).grid(row=0, column=8, padx=6)

        ttk.Label(top, text="Auto refresh (ms):").grid(row=2, column=0, sticky="e", padx=6)
        ttk.Entry(top, textvariable=self.refresh_ms, width=10).grid(row=2, column=1, sticky="w", padx=6)
        ttk.Checkbutton(top, text="Auto-poke when Flag != 1", variable=self.auto_poke).grid(row=2, column=2, columnspan=2, sticky="w", padx=6)

        # Middle: two panes (Afferents, Efferents)
        mid = ttk.PanedWindow(self, orient=tk.HORIZONTAL)
        mid.pack(fill="both", expand=True, padx=8, pady=4)

        # Afferents frame
        af_frame = ttk.LabelFrame(mid, text="Afferents (Sensors)")
        self.af_tree = ttk.Treeview(af_frame, columns=("type", "desc", "path", "value"), show="headings", height=12)
        for c, w in (("type", 90), ("desc", 260), ("path", 320), ("value", 120)):
            self.af_tree.heading(c, text=c.upper())
            self.af_tree.column(c, width=w, stretch=True)
        self.af_tree.pack(fill="both", expand=True, padx=6, pady=6)
        mid.add(af_frame, weight=1)

        # Efferents frame
        ef_frame = ttk.LabelFrame(mid, text="Efferents (Actuators)")
        self.ef_tree = ttk.Treeview(ef_frame, columns=("pin", "path", "state", "controls"), show="headings", height=12)
        for c, w in (("pin", 60), ("path", 360), ("state", 80), ("controls", 220)):
            self.ef_tree.heading(c, text=c.upper())
            self.ef_tree.column(c, width=w, stretch=True)
        self.ef_tree.pack(fill="both", expand=True, padx=6, pady=6)
        mid.add(ef_frame, weight=1)

        # Controls under efferents (buttons mapped to selected row)
        ef_ctrl = ttk.Frame(self)
        ef_ctrl.pack(fill="x", padx=8, pady=4)
        ttk.Button(ef_ctrl, text="Set 0 (OFF)", command=lambda: self._set_selected_actuator(0)).pack(side="left", padx=3)
        ttk.Button(ef_ctrl, text="Set 1 (ON)",  command=lambda: self._set_selected_actuator(1)).pack(side="left", padx=3)
        ttk.Button(ef_ctrl, text="Toggle",     command=self._toggle_selected_actuator).pack(side="left", padx=3)
        ttk.Button(ef_ctrl, text="Open selected file…", command=self._open_selected_file).pack(side="left", padx=12)

        # Status bar
        self.status = tk.StringVar(value="Ready.")
        ttk.Label(self, textvariable=self.status, anchor="w").pack(fill="x", padx=8, pady=(0,8))

        # Seed tables
        self._rebuild_tables()

    def _rebuild_tables(self):
        # Afferents
        for i in self.af_tree.get_children(): self.af_tree.delete(i)
        for a in self.afferents:
            self.af_tree.insert("", "end", values=(a["type"], a["desc"], str(a["path"]), ""))

        # Efferents
        for i in self.ef_tree.get_children(): self.ef_tree.delete(i)
        for e in self.efferents:
            self.ef_tree.insert("", "end", values=(e["pin"], str(e["path"]), "", "Set 0 / Set 1 / Toggle"))

    # ---------- Actions ----------
    def reload_pinout(self):
        self.afferents, self.efferents = parse_pinout(PINOUT_CFG)
        self._rebuild_tables()
        self.status.set("Reloaded pinout.cfg")

    def _open_repo_root(self):
        path = filedialog.askdirectory(initialdir=str(Path.cwd()), title="Open repository root")
        if path:
            os.chdir(path)
            self.status.set(f"Changed working dir → {path}")

    def manual_tick(self):
        """Write a simple eval control script and set the flag to 1."""
        label = self.eval_label.get().strip() or "gui"
        try:
            threshold = float(self.eval_threshold.get())
        except Exception:
            threshold = 1.1

        script = "\n".join([
            "@gather_Input",
            "@shift_Data",
            "@encode",
            f"eval {label} {threshold}",
            "gather_Output",
            ""
        ])
        write_atomic(CONTROL_PANEL_FILE, script + "\n")
        write_atomic(CONTROL_PANEL_FLAG_FILE, "1\n")
        self.status.set("Wrote Control_Panel.ssv and set Control_Panel_Flag.ssv to 1")

    def clear_flag(self):
        write_atomic(CONTROL_PANEL_FLAG_FILE, "0\n")
        self.status.set("Set Control_Panel_Flag.ssv to 0")

    def mark_finished(self):
        write_atomic(CONTROL_PANEL_FINISHED, "1\n")
        self.status.set("Wrote Control_Panel_Finished.ssv = 1")

    def _set_selected_actuator(self, val: int):
        sel = self.ef_tree.selection()
        if not sel:
            messagebox.showinfo("Actuator", "Select an actuator row first.")
            return
        row = self.ef_tree.item(sel[0])["values"]
        path = Path(row[1])
        write_atomic(path, f"{val}\n")
        self.status.set(f"Wrote {val} to {path}")

    def _toggle_selected_actuator(self):
        sel = self.ef_tree.selection()
        if not sel:
            messagebox.showinfo("Actuator", "Select an actuator row first.")
            return
        row = self.ef_tree.item(sel[0])["values"]
        path = Path(row[1])
        cur = safe_read_text(path, "0")
        try:
            nxt = 0 if int(cur) == 1 else 1
        except Exception:
            nxt = 1
        write_atomic(path, f"{nxt}\n")
        self.status.set(f"Toggled {path} → {nxt}")

    def _open_selected_file(self):
        tree = self.af_tree if self.af_tree.selection() else self.ef_tree
        sel = tree.selection()
        if not sel:
            messagebox.showinfo("Open", "Select a row in Afferents or Efferents first.")
            return
        row = tree.item(sel[0])["values"]
        # path is column index 2 for afferents, 1 for efferents
        path_str = row[2] if tree is self.af_tree else row[1]
        path = Path(path_str)
        try:
            if sys.platform.startswith("linux"):
                os.system(f"xdg-open '{path.parent}' >/dev/null 2>&1 &")
            elif sys.platform == "darwin":
                os.system(f"open '{path.parent}'")
            else:
                os.startfile(path.parent)  # type: ignore
        except Exception as e:
            messagebox.showerror("Open", f"Could not open file manager: {e}")

    def _help_paths(self):
        msg = (
            "This GUI expects to run in the same directory as:\n"
            "  • pinout.cfg\n"
            "  • Control_Panel.ssv / Control_Panel_Flag.ssv\n"
            "  • IO_Files/A/*.ssv (sensor values)\n"
            "  • IO_Files/E/*.ssv (actuator commands)\n\n"
            "Use the File → Reload pinout.cfg after changes."
        )
        messagebox.showinfo("Paths", msg)

    # ---------- Polling ----------
    def _refresh(self):
        # Update flag and control script preview
        flag = safe_read_text(CONTROL_PANEL_FLAG_FILE, "(missing)")
        self.flag_val.config(text=flag)

        cp_text = safe_read_text(CONTROL_PANEL_FILE, "")
        cp_preview = cp_text.splitlines()[0] if cp_text else "—"
        if len(cp_preview) > 60:
            cp_preview = cp_preview[:60] + "…"
        self.cp_preview.config(text=cp_preview)

        # Optionally auto-poke (if some other component isn't doing it)
        if self.auto_poke.get() and flag != "1":
            # Minimal script: just eval; Gaia-side scripts can be richer
            script = "\n".join([
                "@gather_Input",
                "@shift_Data",
                "@encode",
                f"eval {self.eval_label.get().strip() or 'gui'} {float(self.eval_threshold.get()):.3f}",
                "gather_Output",
                ""
            ])
            write_atomic(CONTROL_PANEL_FILE, script + "\n")
            write_atomic(CONTROL_PANEL_FLAG_FILE, "1\n")

        # Refresh afferent values
        for item in self.af_tree.get_children():
            vals = self.af_tree.item(item)["values"]
            path = Path(vals[2])
            txt = safe_read_text(path, "")
            # keep it compact
            if len(txt) > 28:
                txt = txt[:28] + "…"
            self.af_tree.set(item, "value", txt)

        # Refresh efferent states
        for item in self.ef_tree.get_children():
            vals = self.ef_tree.item(item)["values"]
            path = Path(vals[1])
            txt = safe_read_text(path, "")
            state = "—"
            try:
                state = "ON" if int(txt) == 1 else "OFF"
            except Exception:
                state = "?"
            self.ef_tree.set(item, "state", state)

        # Schedule next refresh
        try:
            interval = max(100, int(self.refresh_ms.get()))
        except Exception:
            interval = DEFAULT_REFRESH_MS
        self.after(interval, self._refresh)

# ------------------------------------------------------------
if __name__ == "__main__":
    try:
        app = GaiaGUI()
        app.mainloop()
    except KeyboardInterrupt:
        pass
