# Terrafirma Rovers - Visualization

A real-time 3D visualization and control system for autonomous rovers with a sci-fi styled UI.

## Features

- **Real-time 3D visualization** of 5 rovers and their LiDAR point clouds
- **Terrain mapping** from accumulated LiDAR data
- **Sci-fi themed UI** with holographic styling
- **Camera controls**: Free-fly and rover-follow modes
- **Rover control**: Monitor status and button states
- **Height-based coloring** for point clouds and terrain

## Requirements

### macOS
```bash
brew install glfw glm
```

### Ubuntu/Debian
```bash
sudo apt-get install libglfw3-dev libglm-dev
```

## Building

```bash
cd visualization
mkdir build && cd build
cmake ..
make -j4
```

## Running

1. First, start the rover emulators from the project root:
```bash
cd /path/to/terrafirma-rovers
make run
```

2. Then, in a new terminal, run the visualization:
```bash
cd visualization/build
./terrafirma_viz
```

## Controls

| Key/Action | Description |
|------------|-------------|
| Right-click + drag | Look around |
| WASD | Move camera |
| Space / Shift | Move up / down |
| 1-5 | Select rover |
| F | Toggle follow mode |
| F11 | Toggle fullscreen |
| ESC | Release mouse / Exit |

## UI Panels

- **ROVER FLEET** (left): List of all rovers with status
- **ROVER STATUS** (right): Selected rover details and button states
- **RENDER OPTIONS** (right): Terrain and point cloud display settings
- **SYSTEM** (bottom): FPS, point count, and controls help

## Architecture

- **Network Layer**: UDP receiver for pose, LiDAR, and telemetry data
- **Data Management**: Thread-safe storage for rover data and point clouds
- **Rendering**: OpenGL 3.3 with custom shaders
- **UI**: Dear ImGui with custom sci-fi theme

## Dependencies

- GLFW3: Window and OpenGL context
- GLM: Mathematics library
- Dear ImGui: User interface (included)
- GLAD: OpenGL loader (included)

