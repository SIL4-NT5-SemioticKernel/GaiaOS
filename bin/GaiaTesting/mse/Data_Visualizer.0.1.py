import matplotlib.pyplot as plt
import numpy as np
import glob

def read_data(file_path):
    """Function to read data from a given file."""
    with open(file_path, 'r') as f:
        data = np.loadtxt(f)
    return data

def plot_data(file_paths):
    """Function to plot data from multiple files."""
    plt.figure(figsize=(10, 6))  # Adjust size as needed
    for file_path in file_paths:
        data = read_data(file_path)
        x = np.arange(len(data))  # Assuming x depth is consecutive
        plt.plot(x, data, label=file_path)
    
    plt.xlabel('Time')
    plt.ylabel('value')
    plt.title('GaiaOS Test Visualizerman')
    plt.legend()
    plt.grid(True)
    plt.show()

# Find all files matching the pattern
file_paths = glob.glob('*.ssv')

plot_data(file_paths)
