# FreeGLUT/GLUT + GLEW CMake Template

Cross-platform CMake template for OpenGL applications. Uses native GLUT on macOS, FreeGLUT on Linux.

## Prerequisites

**macOS:**
```bash
brew install glew cmake
```
Note: Uses native GLUT framework - no additional installation needed.

**Linux (Ubuntu/Debian):**
```bash
sudo apt-get install freeglut3-dev libglew-dev cmake build-essential
```

**Linux (Fedora/RHEL):**
```bash
sudo dnf install freeglut-devel glew-devel cmake gcc-c++
```

## Build

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

Or use the script:
```bash
./scripts/build.sh
```

## Run

```bash
./output/FreeGLUT_Template
```

## Clean

```bash
cmake --build build --target clean-project
```

Removes `build/`, `output/`, and generated scripts.

## Controls

- **R** - Toggle rotation
- **SPACE** - Reset rotation
- **ESC** - Exit

## Platform Configuration

**macOS:** Uses native GLUT framework by default. Stable with GLEW support. OpenGL 2.1.

**Linux:** Uses FreeGLUT. Full GLEW support.

## Why NOT XQuartz on macOS?

**Default configuration uses native GLUT because:**

1. **Stability:** Native GLUT works reliably with GLEW; FreeGLUT via XQuartz causes segmentation faults
2. **No Dependencies:** No need to install XQuartz/X11
3. **Better Performance:** Uses Metal backend instead of X11 translation layer
4. **Native Integration:** Direct macOS framework, better compatibility

**XQuartz is optional:** Only needed if you specifically require X11 features:
```bash
cmake -DUSE_XQUARTZ=ON ..
```

## CMake Options

```bash
# Use FreeGLUT instead of native GLUT (macOS)
cmake -DUSE_FREEGLUT=ON ..

# Debug build
cmake -DCMAKE_BUILD_TYPE=Debug ..
```

## Why FreeGLUT/GLUT?

Recommended for:
- Learning OpenGL basics
- Traditional OpenGL (fixed pipeline)
- Educational purposes
- Simple, stable windowing

## Related

- GLFW template for modern OpenGL
- FreeGLUT: http://freeglut.sourceforge.net/
- GLEW: http://glew.sourceforge.net/
