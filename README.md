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

**Notes**:

1. This is setup right now, though fully customizable and changeable, for building a copy of the parallel project "MapSimTextServ" and merging the directories. GaiaOS is configurable through the files, and we are working to integrate all of the low level controls you'll see in the code into the scripting engine. You can use this as a standalone application and use the text file (and ssv for the system files) for input and output using batch scripting, python, or anything that can use file based IO.

2. The output commands are mostly not hooked up at the moment, currently core functionality. The priority for focus is going to be on implementing all of the functionality and updating the scripting engine. 

@@ Outro

If something doesn't work or there is a discrepancy please let me know at briarfisk@gmail.com so the issue can be investigated and the reason figured out. 