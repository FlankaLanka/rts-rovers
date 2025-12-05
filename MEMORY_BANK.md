# TerraFirma Rovers - Memory Bank

## Project Overview
Real-time 3D visualization application for monitoring and commanding 5 autonomous rovers. Built with C++17, OpenGL 3.3+, Dear ImGui, and GLFW.

---

## Critical Technical Details

### Coordinate System
**Y is UP (height), X and Z are horizontal.**

Evidence from data analysis:
- X spread: ~415 units (horizontal movement)
- Y spread: ~56 units (small = height variation)
- Z spread: ~281 units (horizontal forward)
- LiDAR scans show Y spread of only 9 units (flat terrain) vs 55-82 units for X/Z

### Data Format
Each `.dat` file contains one frame per line:
```
x,y,z,qx,qy,qz,qw;lidar_x1,lidar_y1,lidar_z1;lidar_x2,lidar_y2,lidar_z2;...
```
- First 7 values: pose (position xyz + quaternion orientation)
- Remaining: LiDAR points (variable count, ~400-600 per frame)

### UDP Ports (per rover)
| Rover | Pose Port | LiDAR Port | Command Port | Telemetry Port |
|-------|-----------|------------|--------------|----------------|
| 1     | 5001      | 5101       | 5201         | 5301           |
| 2     | 5002      | 5102       | 5202         | 5302           |
| 3     | 5003      | 5103       | 5203         | 5303           |
| 4     | 5004      | 5104       | 5204         | 5304           |
| 5     | 5005      | 5105       | 5205         | 5305           |

### Packet Structures
```cpp
// Pose: 28 bytes
struct PosePacket {
    float x, y, z;           // Position
    float qx, qy, qz, qw;    // Quaternion orientation
};

// LiDAR Header: 8 bytes
struct LidarPacketHeader {
    uint32_t scan_id;
    uint16_t chunk_index;
    uint16_t total_chunks;
};

// LiDAR Point: 12 bytes
struct LidarPoint {
    float x, y, z;
};

// Button Command: 2 bytes
struct ButtonCommand {
    uint8_t button_id;    // 0-3
    uint8_t new_state;    // 0 or 1
};

// Telemetry: 4 bytes
struct VehicleTelem {
    uint8_t buttons[4];   // States of buttons 0-3
};
```

---

## Architecture

### Directory Structure
```
terrafirma-rovers/
├── emulator/           # Rover emulator (provided)
├── data/               # Compressed rover data files
├── visualization/      # Main application
│   ├── src/
│   │   ├── core/       # Application, Timer
│   │   ├── data/       # DataManager, PointCloud, RoverData
│   │   ├── network/    # UDPReceiver, PacketParser
│   │   ├── render/     # Camera, Renderer, PointCloudRenderer, etc.
│   │   └── ui/         # UIManager, SciFiTheme
│   ├── include/        # common.h
│   └── external/       # glad, imgui
```

### Threading Model
- **Main Thread**: Rendering, UI, input handling
- **Network Thread**: UDP reception for all 5 rovers (non-blocking sockets)
- **Synchronization**: Mutex-protected data structures, atomic flags

### Key Classes
| Class | Purpose |
|-------|---------|
| `Application` | Main loop, window management, input |
| `DataManager` | Central data hub, terrain grid |
| `UDPReceiver` | Network I/O for all rovers |
| `PointCloud` | Thread-safe LiDAR point storage |
| `Camera` | Free-fly camera with Y-up |
| `Renderer` | Orchestrates all rendering |

---

## Resolved Issues

### 1. Point Cloud Flickering
**Problem**: Frequent GPU buffer updates caused visual flickering.
**Solution**: Incremental GPU uploads - only new points are uploaded, buffer pre-allocated for 2M points.

### 2. Coordinate System
**Problem**: Initial assumption was Z-up, but data uses Y-up.
**Solution**: Changed all shaders and renderers to use Y for height calculations.

### 3. Rovers Showing Offline
**Problem**: `glfwGetTime()` called from network thread (not thread-safe).
**Solution**: Replaced with `std::chrono::steady_clock`.

### 4. Terrain Z-Fighting
**Problem**: Solid and wireframe terrain flickered.
**Solution**: Applied `glPolygonOffset(1.0f, 1.0f)`.

### 5. Camera Movement
**Problem**: WASD was world-relative, not camera-relative.
**Solution**: Movement uses camera's `m_front` and `m_right` vectors directly.

---

## Controls

| Input | Action |
|-------|--------|
| W/S | Move forward/backward (camera direction) |
| A/D | Strafe left/right |
| Space/Shift | Move up/down (world Y) |
| Right-click + drag | Look around |
| Scroll | Zoom |
| 1-5 | Select rover |
| F | Follow selected rover |

---

## Build & Run

### Prerequisites (macOS)
```bash
brew install cmake glfw glm
```

### Build
```bash
cd visualization
mkdir build && cd build
cmake ..
make -j4
```

### Run
```bash
# Terminal 1: Start emulators
cd terrafirma-rovers
make extract  # First time only
make run

# Terminal 2: Start visualization
cd visualization/build
./terrafirma_viz
```

---

## Design Decisions

1. **Platform**: macOS first, Linux later
2. **Rover visuals**: Simple geometric shapes (colored)
3. **Point cloud coloring**: Height gradient (blue→cyan→green→yellow→red)
4. **Terrain**: Toggleable solid/wireframe with height coloring
5. **Point retention**: Keep all points (no decay)
6. **Offline rovers**: Show at last known position with point cloud
7. **Window**: 1280x720, resizable, fullscreen supported
8. **UI theme**: Sci-fi (dark with cyan/orange accents)

---

## Future Improvements

- [ ] Actual 3D rover models
- [ ] Linux compatibility testing
- [ ] Point cloud decimation for performance
- [ ] Terrain LOD system
- [ ] Waypoint/path visualization
- [ ] Recording/playback functionality

---

## Git Repository
`git@github.com:FlankaLanka/rts-rovers.git`

---

*Last updated: December 5, 2025*

