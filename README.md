# GaiaOS: Adaptive Neural Control System

GaiaOS is an advanced, black-box environmental control system leveraging symbolic-connectionist neural networks for robust homeostasis and predictive correction. It autonomously learns and adapts to maintain user-defined goal states within complex systems.

## Key Features
- **Type-Agnostic I/O Interface**: Flexible handling of diverse inputs and outputs.
- **Symbolic Neural Network Core**: Transparent, lossless, and deterministic encoding/decoding of multi-modal data.
- **Fault-Tolerant System**: Manages missing signals with NULL states and ensures reliable operation.
- **Predictive Corrections**: Counterfactual analysis and deviation mapping generate precise corrective strategies.
- **Hierarchical Modularity**: Stackable network modules for scalable complexity.
- **Temporal Dynamics**: Built-in time-series analysis for adaptive learning.

## How It Works
1. Inputs are processed into symbolic representations via atomic primitives.
2. Encoded symbols are combined into hierarchical networks for pattern recognition and inference.
3. Predictive projections compare current and desired system trajectories to identify deviations.
4. Corrective signals are inferred and output for external system adjustment.

## Experimentation Insights
- **Guided vs. Random Training**: Directed exploration stabilizes learning, while random methods improve adaptability.
- **Boredom Mechanism**: Enables exploration by triggering when no new strategies emerge.
- **Future Enhancements**: Integration of transformer-inspired association mechanisms and attention weighting for sensor deviations.

## Tech Stack
- **Languages**: C++, Python for visualization of output data, Python is not needed to run the core engine.
- **Core Algorithms**: Perceptron-based neural processing, counterfactual deviation mapping
- **Simulation Tools**: Deterministic diffusion simulators for robust testing (MapSimTextServ as a standalone application found at https://github.com/NT4-Wildbranch/MapSimTextServ


## Setting up and running an experiment with GaiaOS and viewing the results:

**Steps**:

1. Clone the repository to your local machine.

2. Navigate to the repository.

3. Run builderman.bat [Windows] or builderman.sh [Linux/Mac]

4. Go to the "bin" folder and run "NT4.exe"

5. From there use the "./Scripts/update.txt", "./control_panel.ssv", and "./autoexec.ssv" to control the server. Update runs every tick of the engine. Control panel runs when the "./Control_Panel_Flag.ssv" has '1' written to it. Autoexec runs on startup, this is the bootloader.

# User Manual for Command Interface

## Overview
This document provides an overview of the available commands that can be executed via the file-based interface. Commands control various aspects of the system, such as data handling, input/output management, and evaluation. Commands are issued through a command file and processed accordingly.

---

## 1. Meta Commands
These commands control the command interface itself.

| Command | Description |
|---------|-------------|
| `exit` | Exits the application. |
| `/?`, `help` | Displays the help text. |

---

## 2. File Output Commands
Commands related to writing data to output files.

| Command | Description |
|---------|-------------|
| `clear_Output` | Clears the output file content. |
| `write_Newline` | Appends a newline to the output file. |
| `write_Text <text>` | Writes the specified string to the output file. |
| `write_String` | Writes a string to the output file. |
| `echo <text>` | Prints the specified text to the console. |

---

## 3. Registration Commands
Commands for registering inputs and outputs.

| Command | Description |
|---------|-------------|
| `register_afferent` | Registers a new afferent (input) connection. |
| `register_efferent` | Registers a new efferent (output) connection. |

---

## 4. Homeostasis Module Commands
Commands related to system state management and data handling.

| Command | Description |
|---------|-------------|
| `setup_Gaia <Chrono_Depth> <Afferent_Count> <Efferent_Count> <Start> <End> <Step>` | Initializes the homeostasis module with specific parameters. |
| `shift_Data` | Shifts internal data buffers. |
| `@shift_Data` | Shifts internal data buffers without updates. |
| `set_Afferent_Value <index> <value>` | Sets a value for a specific afferent input. |
| `@set_Afferent_Value <index> <value>` | Sets a value for a specific afferent input without updates. |
| `set_Efferent_Value <index> <value>` | Sets a value for a specific efferent output. |
| `@set_Efferent_Value <index> <value>` | Sets a value for a specific efferent output without updates. |
| `gather_Input` | Collects input data. |
| `@gather_Input` | Collects input data without updates. |
| `gather_Output` | Collects output data. |
| `eval <file_name> <score_threshold_modifier>` | Evaluates the system state based on gathered input. |

**`eval` Argument Explanation:**
- `<score_threshold_modifier>` adjusts how strict or lenient the evaluation is.
- A higher value filters out weaker signals.
- A lower value allows more signals to be considered valid.
- Recommended values: 1.0 - 1.5 for standard evaluations.

Example Usage:
```
eval "output_data" 1.2  # Strict evaluation
eval "output_data" 0.8  # Lenient evaluation
```

---

## 5. Advanced Module Commands
Commands for advanced data processing and analysis.

| Command | Description |
|---------|-------------|
| `write_Deviation_Mapping` | Writes deviation mapping to a file. |
| `set_TSG_Prediction_Params` | Sets prediction parameters for the time series generator. |
| `set_TSG_Deviation_Params` | Sets deviation parameters for the time series generator. |
| `output_Current_Projection` | Outputs the current projection. |
| `output_Deviation_Mapping` | Outputs deviation mapping. |
| `output_Backpropagated_Symbols_Float` | Outputs backpropagated symbols in float format. |

---

## 6. Training and Evaluation Commands
Commands for training and evaluating the system.

| Command | Description |
|---------|-------------|
| `Train` | Trains the system on the latest input set. |
| `Cogitate <filename> <score>` | Evaluates an input set and outputs corrective strategies. |

### Training Steps
1. Set input files in `./IO_Files/A/*.a.ssv`.
2. Run `Train` to load and train on this data.
3. Use `set_Afferent_Value` and `set_Efferent_Value` for manual configuration.
4. Example scripts can be found in `./Scripts/train_0.txt`.

### Cogitate Output
- Generates corrective strategies.
- Output files are saved in `./IO_Files/E/*.e.ssv`.

---

## 7. Utility Commands
Commands for general system usage and information retrieval.

| Command | Description |
|---------|-------------|
| `help` | Displays this help text. |
| `exit` | Exits the application. |

---

## Usage
Commands are provided via a file-based interface, meaning they are issued through an external file and read by the system for execution. Arguments must be correctly formatted for successful execution.

For additional details, refer to the respective command descriptions above.

**Notes**:

1. This is setup right now, though fully customizable and changeable, for building a copy of the parallel project "MapSimTextServ" and merging the directories. GaiaOS is configurable through the files, and we are working to integrate all of the low level controls you'll see in the code into the scripting engine. You can use this as a standalone application and use the text file (and ssv for the system files) for input and output using batch scripting, python, or anything that can use file based IO.

2. The output commands are mostly not hooked up at the moment, currently core functionality. The priority for focus is going to be on implementing all of the functionality and updating the scripting engine. 

@@ Outro

If something doesn't work or there is a discrepancy please let me know at briarfisk@gmail.com so the issue can be investigated and the reason figured out. 
