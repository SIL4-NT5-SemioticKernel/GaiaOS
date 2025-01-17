import os
import sys
from PIL import Image

def make_gif(input_directory, output_filename, duration):
    # Get all the .png files from the input directory
    images = [img for img in os.listdir(input_directory) if img.endswith('.png')]
    images.sort()  # Sort files alphabetically or by any other criteria

    # Ensure there are images to process
    if not images:
        print("No .png files found in the directory.")
        sys.exit(1)

    # Load images
    frames = [Image.open(os.path.join(input_directory, image)) for image in images]

    # Save as GIF
    frames[0].save(
        output_filename,
        save_all=True,
        append_images=frames[1:], 
        optimize=False,
        duration=duration,
        loop=0
    )

if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("Usage: python script.py <input_directory> <output_filename> <duration>")
        sys.exit(1)

    input_directory = sys.argv[1]
    output_filename = sys.argv[2]
    duration = int(sys.argv[3])

    make_gif(input_directory, output_filename, duration)
