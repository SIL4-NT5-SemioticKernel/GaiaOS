#!/usr/bin/env python3
import os
import tkinter as tk
from tkinter import ttk, messagebox

# ---------------------------------------------------------------------------
# Basic paths – tweak these to match your world
# ---------------------------------------------------------------------------

PINOUT_FILE          = "pinout.cfg"
CONTROL_PANEL_FILE   = "Control_Panel.ssv"
CONTROL_PANEL_FLAG   = "Control_Panel_Flag.ssv"   # engine watches this
ONION_CONFIG_FILE    = "values.cfg"           # or "values_table.ssv" etc.
ONION_FLAG_FILE      = "values.flag"         # engine watches this

# ---------------------------------------------------------------------------
# Utility helpers
# ---------------------------------------------------------------------------

def read_first_line(path):
    """Return first line of file or '' if missing/error."""
    try:
        with open(path, "r", encoding="utf-8") as f:
            line = f.readline()
        return line.strip()
    except FileNotFoundError:
        return ""
    except Exception as e:
        return f"<error: {e}>"

def read_whole_file(path):
    """Return file contents or '' (with comment) if missing."""
    try:
        with open(path, "r", encoding="utf-8") as f:
            return f.read()
    except FileNotFoundError:
        return ""
    except Exception as e:
        return f"# error reading {path}: {e}\n"

def write_whole_file(path, text):
    """Write text, creating parent dirs if needed."""
    parent = os.path.dirname(path)
    if parent:
        os.makedirs(parent, exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        f.write(text)

def touch_flag(path, value="1\n"):
    """Write a simple flag (default '1\\n')."""
    try:
        write_whole_file(path, str(value))
        return True
    except Exception as e:
        messagebox.showerror("Flag Error", f"Failed to write flag {path}:\n{e}")
        return False

# ---------------------------------------------------------------------------
# Pinout / I/O parsing (pure-text, no GPIO)
# ---------------------------------------------------------------------------

def parse_pinout(path):
    """
    Parse pinout.cfg-like file and return a list of IO entries.

    Each entry:
      {
        "mode": "A" / "E" / "A1W" / "US" / "PH" / ...,
        "desc": human-friendly description,
        "path": file path for handshake,
        "raw":  original line
      }
    """
    entries = []
    try:
        with open(path, "r", encoding="utf-8") as f:
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
                        # Unknown / future modes: still show them
                        fpath = parts[-1]
                        desc = " ".join(parts[1:-1]) or "(no extra info)"
                except Exception:
                    # malformed line: just show raw
                    fpath = parts[-1] if len(parts) >= 2 else "(?)"
                    desc = f"(malformed) {raw}"

                entries.append({
                    "mode": mode,
                    "desc": desc,
                    "path": fpath,
                    "raw": raw,
                })
    except FileNotFoundError:
        # Missing pinout is allowed; just show nothing.
        pass

    return entries

# ---------------------------------------------------------------------------
# Main UI
# ---------------------------------------------------------------------------

class GaiaConfigUI(tk.Tk):
    def __init__(self):
        super().__init__()

        self.title("GaiaOS Config / I/O Monitor")
        self.geometry("1000x700")

        self._build_widgets()
        self.refresh_all()

    # --- Layout scaffold ---------------------------------------------------

    def _build_widgets(self):
        nb = ttk.Notebook(self)
        nb.pack(fill=tk.BOTH, expand=True)

        self.io_frame    = ttk.Frame(nb)
        self.ctrl_frame  = ttk.Frame(nb)
        self.onion_frame = ttk.Frame(nb)

        nb.add(self.io_frame,    text="I/O State")
        nb.add(self.ctrl_frame,  text="Control Panel")
        nb.add(self.onion_frame, text="Onion / Values")

        self._build_io_tab()
        self._build_ctrl_tab()
        self._build_onion_tab()

        # Status bar
        self.status_var = tk.StringVar(value="Ready.")
        status = ttk.Label(self, textvariable=self.status_var, anchor="w")
        status.pack(fill=tk.X, side=tk.BOTTOM)

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

    def refresh_io(self):
        self.io_tree.delete(*self.io_tree.get_children())
        entries = parse_pinout(PINOUT_FILE)
        for ent in entries:
            val = ""
            if ent["path"] not in ("", "(?)"):
                val = read_first_line(ent["path"])
            self.io_tree.insert("", tk.END,
                                values=(ent["mode"], ent["desc"], ent["path"], val))
        self.status_var.set(f"Loaded {len(entries)} I/O entries from {PINOUT_FILE!r}.")

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

    # --- Onion / values tab -----------------------------------------------

    def _build_onion_tab(self):
        top = ttk.Frame(self.onion_frame)
        top.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)

        ttk.Label(top, text=f"Onion / values config: {ONION_CONFIG_FILE}").pack(anchor="w")

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

        ttk.Button(btn_frame, text="Reload", command=self.load_onion_config).pack(side=tk.LEFT)
        ttk.Button(btn_frame, text="Save",   command=self.save_onion_config).pack(side=tk.LEFT, padx=5)
        ttk.Button(btn_frame, text="Apply (save + flag)",
                   command=self.apply_onion_config).pack(side=tk.LEFT, padx=5)

        self.onion_flag_label = ttk.Label(btn_frame, text="")
        self.onion_flag_label.pack(side=tk.LEFT, padx=10)

    def load_onion_config(self):
        text = read_whole_file(ONION_CONFIG_FILE)
        self.onion_text.delete("1.0", tk.END)
        self.onion_text.insert("1.0", text)
        self.update_onion_flag_label()
        self.status_var.set(f"Loaded {ONION_CONFIG_FILE!r}.")

    def save_onion_config(self):
        text = self.onion_text.get("1.0", tk.END)
        write_whole_file(ONION_CONFIG_FILE, text)
        self.status_var.set(f"Saved {ONION_CONFIG_FILE!r}.")

    def apply_onion_config(self):
        # Save + flip flag → engine can notice and rebuild onion ranges
        self.save_onion_config()
        if touch_flag(ONION_FLAG_FILE, "1\n"):
            self.update_onion_flag_label()
            self.status_var.set(
                f"Saved + flagged onion config ({ONION_CONFIG_FILE!r})."
            )

    def update_onion_flag_label(self):
        if os.path.exists(ONION_FLAG_FILE):
            val = read_first_line(ONION_FLAG_FILE)
            self.onion_flag_label.config(
                text=f"Flag file {ONION_FLAG_FILE}: {val!r}"
            )
        else:
            self.onion_flag_label.config(
                text=f"Flag file {ONION_FLAG_FILE} does not exist."
            )

    # --- global refresh ----------------------------------------------------

    def refresh_all(self):
        self.refresh_io()
        self.load_control_panel()
        self.load_onion_config()
        self.update_ctrl_flag_label()
        self.update_onion_flag_label()


if __name__ == "__main__":
    app = GaiaConfigUI()
    app.mainloop()
