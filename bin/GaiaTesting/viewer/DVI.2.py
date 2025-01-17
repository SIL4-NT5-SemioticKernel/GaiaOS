import os
import sys
import numpy as np
import matplotlib.pyplot as plt
from matplotlib import cm
from matplotlib.colors import LinearSegmentedColormap

# Function to read .ssv file and return a list of rows
def read_ssv_file(filepath):
    data = []
    with open(filepath, 'r') as file:
        for line in file:
            row = list(map(float, line.strip().split()))
            data.append(row)
    return data

# Function to read .lbl file and return label data
def read_lbl_file(filepath):
    labels = {}
    with open(filepath, 'r') as file:
        for line in file:
            parts = line.strip().split()
            label_name = parts[0]
            x = float(parts[1])
            y = float(parts[2])
            size = float(parts[3])
            color = parts[4]
            text = ' '.join(parts[5:])
            labels[label_name] = [x, y, size, color, text]
    return labels

# Function to create and save heatmap
def create_and_save_heatmap(data, custom_colormap, output_filename, labels=None):
    # Step 1: Preprocess the Data
    max_length = max(len(row) - 1 for row in data)
    padded_data = []
    for row in data:
        values = row[1:]
        padded_row = values + [np.nan] * (max_length - len(values))
        padded_data.append(padded_row)
    padded_data = np.array(padded_data, dtype=float)

    fig, ax = plt.subplots()
    masked_data = np.ma.masked_invalid(padded_data)
    cax = ax.imshow(masked_data, cmap=custom_colormap, aspect='auto')
    cbar = fig.colorbar(cax, ax=ax, shrink=0.5, aspect=5)
    ax.set_xlabel('Column Index')
    ax.set_ylabel('Row Index')

    # Add labels if provided
    if labels:
        for label, (x, y, size, color, text) in labels.items():
            ax.text(x, y, text, fontsize=size, color=color, ha='center', va='center', transform=ax.transData)

    plt.savefig(output_filename)
    plt.close(fig)

# Main script execution
if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python script.py <prefix>")
        sys.exit(1)
    
    prefix = sys.argv[1]
    directory = '.'  # current directory
    custom_colormap = LinearSegmentedColormap.from_list("custom_colormap", [(0, "black"), (0.25, "cyan"), (0.5, "purple"), (0.75, "magenta"), (1.0, "lime")])

    for filename in os.listdir(directory):
        if filename.endswith('.ssv'):
            ssv_filepath = os.path.join(directory, filename)
            lbl_filepath = os.path.join(directory, filename[:-4] + '.lbl')
            
            data = read_ssv_file(ssv_filepath)
            output_filename = f"{prefix}_{os.path.splitext(filename)[0]}.png"
            print(output_filename)
            labels = None
            if os.path.exists(lbl_filepath):
                labels = read_lbl_file(lbl_filepath)

            create_and_save_heatmap(data, custom_colormap, output_filename, labels)
