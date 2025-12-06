# Technical Context

## Technology Stack

### Core Technologies
- **C++17/20**: Primary language
- **OpenGL 3.3+**: 3D graphics API
- **GLFW 3.3+**: Window management and input
- **GLM**: Mathematics library (vectors, matrices)
- **Dear ImGui**: Immediate-mode GUI library
- **GLAD**: OpenGL loader

### External Libraries
- **tinyobjloader**: OBJ model loading
- **tinygltf**: GLTF/GLB model loading
- **nlohmann/json**: JSON parsing (for GLTF)
- **stb_image**: Image loading (for GLTF textures)

### Build System
- **CMake 3.16+**: Build configuration
- **Compiler**: Clang/GCC with C++17 support

### Platform Support
- **Primary**: macOS (development)
- **Target**: Linux (Ubuntu 22.04)
- **Hardware**: Standard laptop (ThinkPad T16 Gen 3 equivalent)

## Development Setup

### Dependencies (macOS)
```bash
brew install cmake glfw glm
```

### Dependencies (Linux)
```bash
sudo apt-get install cmake libglfw3-dev libglm-dev
```

### Build Instructions
```bash
cd visualization
mkdir build && cd build
cmake ..
make -j4
./terrafirma_viz
```

## Network Protocol

### UDP Ports (per rover)
- **Pose**: `9000 + RoverID` (9001-9005)
- **LiDAR**: `10000 + RoverID` (10001-10005)
- **Telemetry**: `11000 + RoverID` (11001-11005)
- **Commands**: `8000 + RoverID` (8001-8005)

### Packet Structures
All packets use `#pragma pack(push, 1)` for binary compatibility.

**PosePacket** (32 bytes):
- `double timestamp`
- `float posX, posY, posZ`
- `float rotXdeg, rotYdeg, rotZdeg`

**LidarPacket** (variable):
- Header: `timestamp, chunkIndex, totalChunks, pointsInThisChunk`
- Array: Up to 100 `LidarPoint` (x, y, z)

**VehicleTelem** (9 bytes):
- `double timestamp`
- `uint8_t buttonStates` (bitfield)

**Button Command** (1 byte):
- `uint8_t buttonStates` (bitfield)

## Coordinate System
- **Y-up**: Y is vertical (height), X/Z are horizontal
- **Rover Orientation**: 
  - `rotation.y` = yaw (around Y axis, turning left/right)
  - `rotation.x` = roll (around X axis, unused for construction vehicles)
  - `rotation.z` = pitch (around Z axis, unused for construction vehicles)
- **Forward Direction**: +Z when yaw=0, rotates with yaw

## Performance Considerations
- **Target FPS**: 60+ FPS
- **Network Rate**: 10Hz (100ms between updates)
- **Interpolation**: Exponential smoothing (10.0f factor)
- **Terrain Updates**: Throttled to 0.5s intervals
- **Point Cloud Rendering**: Batch GPU updates, separate buffers per rover
- **Pathfinding**: Grid-based A* (efficient for terrain cells)

## File Structure
```
visualization/
├── CMakeLists.txt
├── include/
│   ├── common.h          # Shared definitions, packet structures
│   └── TimeUtil.h        # Shared time source
├── src/
│   ├── core/             # Application, Timer
│   ├── network/          # UDPReceiver, PacketParser
│   ├── data/             # DataManager, RoverData, PointCloud
│   ├── render/           # Renderer, Shaders, Camera, *Renderer
│   ├── pathfinding/      # AStar, PathRenderer
│   ├── terrain/          # TerrainOperation, TerrainRaycast
│   └── ui/               # UIManager, SciFiTheme
└── assets/
    └── models/
        └── rover.glb     # Custom rover model (optional)
```

## Model Loading
Supports multiple formats (tried in order):
1. `rover.glb` (GLTF binary - recommended)
2. `rover.gltf` (GLTF text)
3. `rover.obj` (Wavefront OBJ)

Models are automatically:
- Scaled to ~15 units (configurable)
- Centered at origin
- Ground-aligned (Y minimum = 0)

## Known Technical Constraints
- **UDP Packet Loss**: No retransmission, handles gracefully
- **Thread Safety**: Careful mutex usage required
- **OpenGL Context**: Must be accessed from main thread only
- **Terrain Sparse**: Some areas may have no data (handled with fallback plane)
- **Pathfinding**: Limited to existing terrain cells (no off-grid movement)

