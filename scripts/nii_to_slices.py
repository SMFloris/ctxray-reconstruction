import nibabel as nib
import numpy as np
import os
import argparse

parser = argparse.ArgumentParser(description="Export NIfTI slices as PGM.")
parser.add_argument("--input", required=True, help="Path to input NIfTI file")
parser.add_argument("--output", required=True, help="Directory for PGM slices")
args = parser.parse_args()

nii = nib.load(args.input)
data = nii.get_fdata()

os.makedirs(args.output, exist_ok=True)

norm = (255 * (data - data.min()) / (data.max() - data.min())).astype(np.uint8)

for i in range(data.shape[2]):
    slice2d = norm[:, :, i]
    if slice2d.max() == 0:
        continue

    slice2d = np.rot90(slice2d, k=3)
    h, w = slice2d.shape
    size = max(h, w)

    # create square canvas
    square = np.zeros((size, size), dtype=np.uint8)
    y = (size - h) // 2
    x = (size - w) // 2
    square[y : y + h, x : x + w] = slice2d

    path = os.path.join(args.output, f"slice_{i:04d}.pgm")
    with open(path, "wb") as f:
        f.write(f"P5\n{size} {size}\n255\n".encode())
        f.write(square.tobytes())
