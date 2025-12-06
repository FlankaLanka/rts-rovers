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
posX,posY,posZ,rotX,rotY,rotZ;lidar_x1,lidar_y1,lidar_z1;lidar_x2,lidar_y2,lidar_z2;...
```
- First 6 values: pose (position xyz + rotation in degrees)
- Remaining: LiDAR points (variable count, ~400-600 per frame)

### UDP Ports (per rover)
| Rover | Pose Port | LiDAR Port | Command Port | Telemetry Port |
|-------|-----------|------------|--------------|----------------|
| 1     | 9001      | 10001      | 11001        | 8001           |
| 2     | 9002      | 10002      | 11002        | 8002           |
| 3     | 9003      | 10003      | 11003        | 8003           |
| 4     | 9004      | 10004      | 11004        | 8004           |
| 5     | 9005      | 10005      | 11005        | 8005           |

### Packet Structures
```cpp
// Pose: 32 bytes
struct PosePacket {
    double timestamp;         // Seconds since emulator start
    float posX, posY, posZ;   // Position
    float rotXdeg, rotYdeg, rotZdeg;  // Rotation in degrees
};

// LiDAR Header: 20 bytes
struct LidarPacketHeader {
    double timestamp;
    uint32_t chunkIndex;
    uint32_t totalChunks;
    uint32_t pointsInThisChunk;
};

// LiDAR Point: 12 bytes
struct LidarPoint {
    float x, y, z;
};

// Button Command: 1 byte
uint8_t buttonStates;  // Bits 0-3 = buttons 0-3

// Telemetry: 9 bytes
struct VehicleTelem {
    double timestamp;
    uint8_t buttonStates;  // Bits 0-3 = buttons 0-3
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
│   │   ├── render/     # Camera, Renderer, PointCloudRenderer, CircleRenderer
│   │   ├── terrain/    # TerrainRaycast, TerrainOperation
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
| `Renderer` | Orchestrates all rendering (5 point cloud renderers) |
| `TerrainOperation` | Dig/pile state machine and logic |
| `TerrainOperationManager` | Manages operations for all rovers |
| `CircleRenderer` | Renders semi-transparent circles on terrain |

---

## Implemented Features

### Engine Control (Button 0)
- **Button 0** controls pause/resume of each rover
- **Green button** = Engine ON (reading data)
- **Red button** = Engine OFF (paused)
- **True pause**: Emulator stops reading .dat file, resumes from same position
- **Visual indicator**: Paused rovers are dimmed and red-tinted
- Click button in UI to toggle state

### Dig/Pile Operations (Button 1 & 2)

#### Button 1 - DIG
Lowers terrain within a user-defined circular area.

#### Button 2 - PILE
Raises terrain within a user-defined circular area.

#### Operation Flow:
1. **Click DIG/PILE button** → Enters DRAWING state
2. **Click and drag on terrain** → Draws circle (center at click, radius at release)
3. **Release mouse** → Enters CONFIRMING state, shows confirm/redo panel
4. **Click CONFIRM** → Rover moves to dig site
5. **Rover arrives** → Operation begins (terrain changes in 5m steps)
6. **Auto-completes** at 25m depth OR **click button again to cancel**

#### Operation Constants:
| Parameter | Value |
|-----------|-------|
| Max Depth | 25m |
| Dig/Pile Rate | 1 m/s |
| Depth Steps | 5m increments |
| Move Speed | 5 m/s |
| Hover Height | 3m above terrain |
| Arrival Threshold | 1m |

#### Technical Details:
- **TerrainRaycast**: Converts mouse screen position to world ray, intersects with terrain grid
- **Retina display fix**: Converts window coords to framebuffer coords for accurate raycasting
- **CircleRenderer**: Renders semi-transparent filled circle (red for dig, orange for pile)
- **Rover control**: When rover is MOVING/OPERATING, UDP pose updates are blocked (`m_roverControlled` flag)
- **Interpolation skip**: Controlled rovers skip position interpolation to prevent fighting

#### State Machine:
```
IDLE → DRAWING → CONFIRMING → MOVING → OPERATING → IDLE
         ↑          │
         └──(redo)──┘
```

### Point Cloud Rendering
- **Separate GPU buffer per rover** to prevent data overwriting
- Incremental uploads (only new points sent to GPU)
- Height gradient coloring (blue→cyan→green→yellow→red)

---

## Resolved Issues

### 1. Point Cloud Disappearing
**Problem**: Single `PointCloudRenderer` shared by all rovers - each rover overwrote previous data.
**Solution**: Created array of 5 `PointCloudRenderer` instances, one per rover.

### 2. Camera Snapping on Click
**Problem**: GLFW cursor warp when enabling capture mode caused sudden camera jump.
**Solution**: Skip first 2 cursor events after enabling capture mode.

### 3. Point Count Updating When Paused
**Problem**: Emulator sent LiDAR even when paused.
**Solution**: Emulator only sends LiDAR when engine is running (button 0 = 1).

### 4. Coordinate System
**Problem**: Initial assumption was Z-up, but data uses Y-up.
**Solution**: Changed all shaders and renderers to use Y for height calculations.

### 5. Rovers Showing Offline
**Problem**: `glfwGetTime()` called from network thread (not thread-safe).
**Solution**: Replaced with `std::chrono::steady_clock`.

### 6. Terrain Z-Fighting
**Problem**: Solid and wireframe terrain flickered.
**Solution**: Applied `glPolygonOffset(1.0f, 1.0f)`.

### 7. Camera Movement
**Problem**: WASD was world-relative, not camera-relative.
**Solution**: Movement uses camera's `m_front` and `m_right` vectors directly.

### 8. Rover Movement Jitter
**Problem**: Sensor noise from emulator caused jerky movement.
**Solution**: Exponential smoothing with angle wrapping for rotation.

### 9. Dig Site Location Wrong
**Problem**: Mouse coordinates not converted for Retina displays.
**Solution**: Scale mouse coords by (framebuffer size / window size).

### 10. Rover Not Moving to Dig Site
**Problem**: Interpolation kept overwriting position set by operation.
**Solution**: Skip `interpolate()` when rover is controlled by operation.

---

## Known Issues (TODO)

### 1. FPS Drop with Point Clouds
**Cause**: 
- Frequent GPU buffer uploads (5 rovers × 10Hz = 50 uploads/sec)
- Unlimited point accumulation (millions of points)
- Thread safety issue with data pointer during upload

**Fix needed**:
- Cap max points per rover (~500K, discard oldest)
- Throttle GPU uploads
- Copy data before releasing mutex

### 2. Rover Mesh Wrong Orientation
**Cause**: Rover mesh built with Z as height, but system uses Y-up.
**Fix needed**: Rebuild mesh vertices to use Y as height, cone pointing in +Z.

### 3. Rotation Axes Incorrect
**Cause**: Rotation order assumes Z-up system.
**Fix needed**: For Y-up, yaw should rotate around Y axis.

---

## Controls

| Input | Action |
|-------|--------|
| W/S | Move forward/backward (camera direction) |
| A/D | Strafe left/right |
| Space | Move up (world Y) |
| Right-click + drag | Look around |
| Scroll | Zoom |
| 1-5 | Select rover |
| F | Follow selected rover |
| F11 | Toggle fullscreen |
| ESC | Cancel operation / Release mouse / Exit |

### Dig/Pile Controls
| Input | Action |
|-------|--------|
| DIG button | Start dig operation (or cancel if digging) |
| PILE button | Start pile operation (or cancel if piling) |
| Left-click + drag | Draw circle on terrain |
| CONFIRM | Start rover movement to site |
| REDO | Redraw circle |
| STOP/CANCEL | Cancel operation |

---

## Build & Run

### Prerequisites (macOS)
```bash
brew install cmake glfw glm
```

### Build
```bash
# Build emulator
cd terrafirma-rovers
make

# Build visualization
cd visualization
mkdir build && cd build
cmake ..
make -j4
```

### Run
```bash
# Terminal 1: Start emulators
cd terrafirma-rovers
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
5. **Point retention**: Keep all points (no decay) - causes FPS issues
6. **Offline rovers**: Show at last known position with point cloud
7. **Window**: 1280x720, resizable, fullscreen supported
8. **UI theme**: Sci-fi (dark with cyan/orange accents)
9. **Engine control**: Button 0 = true pause (emulator stops reading file)
10. **Dig/Pile**: 25m max depth, 5m step increments, 1 m/s rate

---

## Future Improvements

- [ ] Fix rover mesh for Y-up coordinate system
- [ ] Fix rotation axes for Y-up
- [ ] Add point cloud culling/max cap for performance
- [ ] Actual 3D rover models
- [ ] Linux compatibility testing
- [ ] Terrain LOD system
- [ ] Waypoint/path visualization
- [ ] Recording/playback functionality
- [ ] Button 3 functionality (TBD)

---

## Git Repository
`git@github.com:FlankaLanka/rts-rovers.git`

---

*Last updated: December 6, 2025*
