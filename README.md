# SaarPI Project

A WebAssembly-based medical imaging visualization application built with Raylib, featuring algebraic reconstruction techniques for tomographic data.

## Features

- **WebAssembly Support**: Runs directly in the browser using Emscripten
- **Desktop Version**: Native desktop executable for development and testing
- **Interactive UI**: Built with Raylib for smooth graphics and user interaction
- **Tomographic Reconstruction**: Implements iterative methods like Kaczmarz algorithm

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
This will serve the application using emrun.

### Cleaning

Remove build artifacts:
```bash
make clean
```

## Project Structure

- `src/`: Source code
  - `main.c`: Main application entry point
  - `ui.h`: User interface components
  - `art.h`: Algebraic reconstruction techniques
- `libs/`: External libraries (Raylib WebAssembly)
- `result/`: Build output directory
- `resources/`: Asset files and sample data

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- [Raylib](https://www.raylib.com/) - Simple and easy-to-use game programming library
- [Emscripten](https://emscripten.org/) - Compiler toolchain for WebAssembly