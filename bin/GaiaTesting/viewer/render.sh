#!/bin/bash

# Delete .ssv, .lbl, and .png files
rm -f *.ssv
rm -f *.lbl
rm -f *.png

# Copy specific files from parent directory
cp ../*.MAP.ssv ./*.ssv
cp ../*.MAP.lbl ./*.lbl

# Append zeros to 0001.ssv
for i in {1..26}; do
    echo "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0" >> 0001.ssv
done

# Append zeros to 0000.ssv
for i in {1..26}; do
    echo "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0" >> 0000.ssv
done

# Prompt the user for an experiment name
read -p "Enter Experiment Name: " EName

# Execute Python scripts and commands
python3 DVI.2.py "$EName"
python3 DC.0.py . "${EName}.fr10.gif" 10
# Uncomment the following lines to enable additional commands
# python3 DC.0.py . "${EName}.fr25.gif" 25
# python3 DC.0.py . "${EName}.fr50.gif" 50
# python3 DC.0.py . "${EName}.fr100.gif" 100
# python3 DC.0.py . "${EName}.fr150.gif" 150
# python3 DC.0.py . "${EName}.fr200.gif" 200
# python3 DC.0.py . "${EName}.fr250.gif" 250
# python3 DC.0.py . "${EName}.fr500.gif" 500

# Output the resulting GIF name
echo "${EName}.fr10.gif"
