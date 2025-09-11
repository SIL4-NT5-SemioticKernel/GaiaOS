Got it — you want a full **GitHub-ready README.md** style manual that incorporates everything we’ve covered: intro, commands, examples, non-obvious quirks, and workflow. I’ll keep Markdown formatting clean (no characters that break rendering, e.g. I’ll use fenced code blocks carefully and avoid stray pipes). This can live directly as `README.md` in your repo.

---

# Gaia Shell Manual

GaiaOS is a **file-driven control shell** for building sensor/actuator loops. Instead of typing into a prompt, you place commands in `Control_Panel.ssv`, flip a flag file, and the engine executes them. It learns patterns over time and decides outputs by writing `1` (ON) or `-1` (OFF) into efferent files.

This manual explains the command language, workflow, and subtle details needed to get GaiaOS working in practice.

---

## 1. Core Concepts

* **Afferents (A)** → inputs.
  Each input is stored in `IO_Files/A/<i>.a.ssv` as a float.

* **Efferents (E)** → outputs.
  Each output is stored in `IO_Files/E/<j>.e.ssv` as an integer: `1` or `-1`.

* **Control panel files**

  * `Control_Panel.ssv` — buffer of commands to run
  * `Control_Panel_Flag.ssv` — write `1` to request execution
  * `Control_Panel_Finished.ssv` — engine writes `1` when finished

* **Eval** → the decision step. Evaluates current inputs, scores candidate traces, and sets outputs.

* **Scripts** → any `.ssv` file in `./Scripts/`. Unknown tokens in the control panel are treated as includes from this directory.

---

## 2. Command Reference

### System

* `help` or `/?` — Show this manual
* `exit` — Quit GaiaOS server loop

### Setup

* `setup_Gaia <Chrono> <A> <E> <Start> <End> <Step>`
  Registers `<A>` afferents and `<E>` efferents, creates IO files, builds onion-layer granulation bands, and sets memory depth `<Chrono>`.
* `register_afferent` / `register_efferent` — Add a single IO channel (no file creation)

### Data Handling

* `@gather_Input` — Read values from IO files into internal state
* `@shift_Data` — Advance buffers / chronology
* `@encode` — Encode the current snapshot
* `eval <label> <threshold>` — Evaluate, decide outputs, and record analytics
* `gather_Output` — Mirror decisions to efferent files (`1` or `-1`)
* `get_Output_Signals <index>` — Print the current output signal for debugging

### Utilities

* `echo … /end/` — Print text to console
* `write_String <path> <token>` — Overwrite file with a single token

### Diagnostics

* `output_AE` — Show IO map
* `output_TSG` — Dump time-series internals
* `output_NNet` — Dump node network
* `output_Scaffolds` — Show scaffold structures
* `output_Current_Projection <RF>` — Show projection for a read-front
* `output_Deviation_Mapping` — Show deviation lanes
* `output_Backpropagated_Symbols_Float` — Show backprop symbols

**Note:** Commands prefixed with `@` are silent variants (suppress banners).

---

## 3. Eval Threshold

`eval <label> <threshold>` uses a dynamic cutoff:

```
threshold = max_score * <threshold_modifier>
```

* If efferent score > threshold → ON (`1`)
* Else → OFF (`-1`)

Guidelines:

* `1.0` = permissive
* `1.2` = balanced
* `>1.5` = strict

---

## 4. Example Workflows

### Basic Setup (2 inputs, 2 outputs)

File: `Scripts/autoexec.ssv`

```
setup_Gaia  8  2  2  50  100  25
output_AE
```

Creates:

* `IO_Files/A/0.a.ssv`, `IO_Files/A/1.a.ssv`
* `IO_Files/E/0.e.ssv`, `IO_Files/E/1.e.ssv`
* Granulation rings at ±0, ±25, ±50, ±75, ±100 around 50

---

### Manual Control (single tick)

```
# Seed inputs
write_String IO_Files/A/0.a.ssv 42
write_String IO_Files/A/1.a.ssv 60

# Run one iteration
@gather_Input
@shift_Data
@encode
eval loop 1.2
gather_Output

# Inspect a decision
get_Output_Signals 0
```

---

### Continuous Operation

File: `Scripts/loop.ssv`

```
@gather_Input
@shift_Data
@encode
eval loop 1.2
gather_Output
```

Each cycle (from your driver or script):

1. Write `loop.ssv` into `Control_Panel.ssv`
2. Set `Control_Panel_Flag.ssv = 1`
3. Wait until `Control_Panel_Finished.ssv = 1`
4. Read outputs from `IO_Files/E/*.e.ssv`
5. Repeat

---

## 5. Non-Obvious Details

### The `[filename]` Argument

* In `eval` and `Cogitate`, `[filename]` is a **label**, not an actual file. It’s used to tag analytics output under `GaiaTesting/`.
* As a standalone token, `[filename]` is a **script include** that must exist in `./Scripts/`.

Default `Testermon.txt` in the Pi driver is just a placeholder label.

---

### Onion-Layer Granulation Bands

Inputs are bucketed into concentric ranges around a target (`Start`):

```
Start=50, End=100, Step=25
[50..50]     exact
[25..75]     ±25
[0..100]     ±50
[-25..125]   ±75
[-50..150]   ±100
```

This “onion” scheme lets the engine score matches as *exact*, *near*, or *far*.

---

### Output File Semantics

`gather_Output` **appends** each decision, e.g.:

```
-1
1
1
-1
```

Your hardware driver should read the **last line** or truncate the file before use.

---

### Training vs Inference

There is no separate `Train` token in this build. Training happens implicitly during:

```
@gather_Input
@shift_Data
@encode
eval ...
```

---

### The Idle `update.txt`

On each idle loop, the engine interprets `Scripts/update.txt`. Keep an empty file there or put harmless comments to avoid error spam.

---

## 6. Quick Start

1. Place `autoexec.ssv` in `./Scripts/` with your setup
2. Run GaiaOS — IO files will be created
3. Write sensor values into `A/*.a.ssv`
4. Trigger `loop.ssv` with the flag handshake
5. Read efferent values from `E/*.e.ssv` to drive hardware

---

## 7. Notes & Caveats

* Unknown tokens are treated as `./Scripts/<token>` or `./Scripts/<token>.txt`
* Outputs are always `1` (ON) or `-1` (OFF)
* Tuning eval threshold is key: too low = chatter, too high = everything stays off
* Use `@` variants to keep logs quiet during tight loops

---




#Image of pinout from https://raspberrypi.stackexchange.com/questions/68126/have-the-gpio-pins-changed-between-the-pi-2-and-the-pi-3