# System Patterns

## Architecture Overview
Multi-threaded application with clear separation between network I/O, data management, and rendering.

```
┌─────────────────┐
│  Network Thread │  ← Receives UDP packets (10Hz)
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  Data Manager   │  ← Thread-safe data storage
│  - Rover States │
│  - Point Clouds │
│  - Terrain Grid │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  Render Thread  │  ← Main loop (60+ FPS)
│  - OpenGL       │
│  - ImGui UI     │
└─────────────────┘
```

## Component Breakdown

### Core Application (`Application.h/cpp`)
- **Responsibility**: Main loop, event handling, component orchestration
- **Key State**:
  - Selected rover (1-5 keys)
  - Camera modes (free, follow, 3rd person)
  - Control modes (manual, RTS, WAY) per rover
  - Pathfinding state per rover
- **Patterns**: Singleton via static pointer, callback-based event handling

### Network Layer (`UDPReceiver.h/cpp`, `PacketParser.h/cpp`)
- **Responsibility**: Receive UDP packets, parse binary data
- **Threading**: Separate network thread, mutex-protected data updates
- **Patterns**: Producer-consumer (network thread → data manager)

### Data Management (`DataManager.h/cpp`, `RoverData.h/cpp`, `PointCloud.h/cpp`)
- **Responsibility**: Store and manage rover state, point clouds, terrain
- **Thread Safety**: Mutexes for shared data, atomic flags for control states
- **Patterns**: Repository pattern, observer pattern (dirty flags for terrain)

### Rendering (`Renderer.h/cpp`, `*Renderer.h/cpp`)
- **Responsibility**: OpenGL rendering, shader management
- **Components**:
  - `RoverRenderer`: Renders rover models (supports OBJ/GLTF/GLB)
  - `PointCloudRenderer`: Renders LiDAR point clouds (separate VBO per rover)
  - `TerrainRenderer`: Renders terrain mesh from height grid
  - `PathRenderer`: Renders A* paths and destination markers
  - `CircleRenderer`: Renders dig/pile operation circles
- **Patterns**: Component-based rendering, shader abstraction

### Pathfinding (`AStar.h/cpp`)
- **Responsibility**: Grid-based A* pathfinding on terrain
- **Algorithm**: 8-directional movement, slope checking, path simplification
- **Patterns**: Graph search, heuristic-based optimization

### Terrain Operations (`TerrainOperation.h/cpp`)
- **Responsibility**: Dig/pile operations, rover movement to sites
- **State Machine**: IDLE → DRAWING → CONFIRMING → MOVING → OPERATING
- **Patterns**: State pattern, command pattern

### UI (`UIManager.h/cpp`, `SciFiTheme.h/cpp`)
- **Responsibility**: ImGui interface, sci-fi styling
- **Panels**: Fleet panel, status panel, settings panel, operation panel
- **Patterns**: Immediate mode GUI, theme singleton

## Data Flow Patterns

### Rover Pose Updates
```
UDP Packet → PacketParser → RoverData::updatePose() 
  → Interpolation (exponential smoothing) 
  → Render
```

### LiDAR Point Cloud Updates
```
UDP Chunks → PacketParser → PointCloud::addPoints() 
  → Batch GPU upload (separate VBO per rover)
  → Render
```

### Terrain Updates
```
LiDAR Points → TerrainGrid::addPoint() 
  → Height map update
  → Dirty flag set
  → Mesh regeneration (throttled to 0.5s)
  → Render
```

### Button Commands
```
UI Click → UDPReceiver::sendCommand() 
  → UDP packet to emulator
  → Emulator updates state
  → Telemetry returns
  → UI updates
```

## Threading Model
- **Main Thread**: Rendering, UI, input handling
- **Network Thread**: UDP receive loop (blocking recv)
- **Synchronization**: 
  - Mutexes for data structures
  - Atomic flags for control states
  - Shared time source (`TimeUtil`) for consistent timestamps

## Design Patterns Used
1. **Singleton**: Application (via static pointer), TimeUtil
2. **Observer**: Terrain dirty flags trigger mesh updates
3. **State Machine**: TerrainOperation states, rover control modes
4. **Strategy**: Different movement strategies (manual, pathfinding, dig/pile)
5. **Component**: Renderer components (Rover, PointCloud, Terrain, etc.)
6. **Repository**: DataManager centralizes data access

## Key Design Decisions
- **Separate PointCloudRenderer per rover**: Prevents GPU buffer conflicts
- **Exponential smoothing**: Frame-rate independent interpolation
- **Terrain update throttling**: Prevents excessive mesh regeneration
- **Grid-based pathfinding**: Uses existing terrain structure, no external library
- **Y-up coordinate system**: Matches data format, standard for terrain

