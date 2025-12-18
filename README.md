# Kaczmarz Reconstruction

A WebAssembly-based interactive demonstration of CT image reconstruction using the Kaczmarz algorithm, built with Raylib and Emscripten. This project implements algebraic reconstruction techniques for tomographic data, specifically demonstrating how CT machines reconstruct 2D slices from X-ray projections.

**Author:** Stoica-Marcu Floris-Andrei, Ovidius University of Constanta

## Overview

CT machines work by shooting thin X-rays that pass through material to a detector plate. By shooting many rays at different angles for a single horizontal slice, the CT machine and reconstruction algorithm can reconstruct the slice. Combining multiple horizontal slices vertically results in a 3D reconstruction.

This application uses brain imaging data from the [OpenNeuro Dataset ds000102](https://github.com/OpenNeuroDatasets/ds000102) for demonstration purposes.

## Features

- **Interactive CT Reconstruction Demo**: Step-by-step visualization of the Kaczmarz algorithm
- **WebAssembly Support**: Runs directly in the browser using Emscripten
- **Desktop Version**: Native desktop executable for development and testing
- **Fan-beam CT Simulation**: Implements a virtual fan-beam circular CT scanner layout
- **Real Dataset Integration**: Uses brain imaging data from OpenNeuro for realistic demonstrations
- **Kaczmarz Algorithm Implementation**: Iterative algebraic reconstruction technique for solving linear systems
- **Line Integral Computation**: Uses Liang-Barsky algorithm for efficient ray-voxel intersection calculations

## Mathematical Background

### X-Ray Attenuation
Each X-ray beam passing through material is attenuated according to Lambert-Beer's law:
```
I_B = I_A * e^(-∫_A^B μ(s) ds)
```
where μ is the linear attenuation coefficient along the path.

### Discretization
The continuous image is divided into a grid of square voxels. Each ray's projection value β is computed as:
```
β = ∑_{i,j} μ_{i,j} · l_{i,j}
```
where l_{i,j} is the length of the ray segment inside voxel (i,j), calculated using the Liang-Barsky line clipping algorithm.

### Kaczmarz Algorithm
The Kaczmarz method iteratively solves the system Ax = b by projecting onto hyperplanes:
```
x^{k+1} = x^k + (b_i - <a_i, x^k>) / ||a_i||² * a_i
```
where a_i is the i-th row of the system matrix A, and b_i is the measured projection.

## Tools

- **Slice to PPM Converter**: Converts .nii medical imaging slices to PPM format for further processing or visualization

## Prerequisites

### For Desktop Build
- GCC/Clang compiler
- Raylib library (system installation or local build)

### For Web Build
- Emscripten SDK
- Make sure `RAYLIB_INCLUDE_PATH` and `RAYLIB_LIB_PATH` environment variables are set

## Installation

1. Clone the repository:
   ```bash
   git clone <repository-url>
   cd saarpi-project
   ```

2. For desktop development, ensure Raylib is installed on your system.

3. For web builds, install Emscripten and set up the environment.

## Usage

### Building

Run all targets (desktop and web):
```bash
make all
```

Build only desktop version:
```bash
make desktop
```

Build only web version:
```bash
make web
```

### Running

Desktop version will run automatically after build.

For web version, use:
```bash
make web-run
```
This will serve the application using emrun. The web demo includes:
- Interactive scrolling through reconstruction stages
- Real-time visualization of the Kaczmarz algorithm convergence
- Educational content explaining CT principles and mathematics

### Cleaning

Remove build artifacts:
```bash
make clean
```

## Project Structure

- `src/`: Source code
  - `main.c`: Main application entry point
  - `ui.h`: User interface components
  - `art.h`: Algebraic reconstruction techniques (ART) implementation
  - `ray.h`: Ray casting and projection calculations
  - `arena.h`: Memory management utilities
  - `utils.h`: General utility functions
  - `resources/`: Asset files and sample data
    - `nii_slices/`: Brain imaging slices in PGM format
    - CT scanner images and algorithm illustrations
- `scripts/`: Build scripts and configuration
- `shell.html`: Web deployment template
- `Makefile`: Build configuration

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- **Special Thanks**: Dean Aurelian Nicola for teaching the CT reconstruction class, and Prof. Dr. Dorin-Mircea Popovici for inspiration through work in the CERVA lab
- [Raylib](https://www.raylib.com/) - Simple and easy-to-use game programming library
- [Emscripten](https://emscripten.org/) - Compiler toolchain for WebAssembly
- [OpenNeuro Dataset ds000102](https://github.com/OpenNeuroDatasets/ds000102) - Brain imaging data used for demonstrations
- [Liang-Barsky Algorithm](https://en.wikipedia.org/wiki/Liang%E2%80%93Barsky_algorithm) - Line clipping implementation
- [Kaczmarz Method](https://en.wikipedia.org/wiki/Kaczmarz_method) - Iterative reconstruction algorithm

## References

- Radon, Johann. "On the determination of functions from their integral values along certain manifolds." IEEE Transactions on Medical Imaging 5 (1986): 170-176.
- Kelly, A.M., et al. "Competition between functional brain networks mediates behavioral variability." Neuroimage 39(1):527-37 (2008).
- Mennes, M., et al. "Inter-individual differences in resting-state functional connectivity predict task-induced BOLD activity." Neuroimage 50(4):1690-701 (2010).
- Liang-Barsky Algorithm on [Wikipedia](https://en.wikipedia.org/wiki/Liang%E2%80%93Barsky_algorithm) and [GeeksforGeeks](https://www.geeksforgeeks.org/computer-graphics/liang-barsky-algorithm/)