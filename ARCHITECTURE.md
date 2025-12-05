# Terrafirma Rovers - System Architecture

## Overview

This document describes the architecture of the real-time 3D visualization and control system for autonomous rovers. The system receives UDP data streams from 5 rovers, visualizes them in a 3D environment, and provides a sci-fi styled control interface.

## Design Decisions

| Decision | Choice | Notes |
|----------|--------|-------|
| **Platform** | macOS first | Linux compatibility later |
| **Rover Models** | Simple geometric shapes | Different colors per rover; models added later |
| **Point Cloud Colors** | Height gradient | Low to high elevation coloring |
| **Terrain Rendering** | Toggleable wireframe/solid | Both support height-based coloring |
| **Point History** | Keep all points | No limit on point cloud history |
| **Offline Rovers** | Show at last position | Keep point cloud visible |
| **Window Size** | 1280x720 default | Resizable, fullscreen toggle |
| **Camera Controls** | WASD + mouse | 1-5 keys for rover selection |
| **Units Display** | Show units in UI | Scale-independent (fixed screen size) |
| **Coordinate Units** | Meters | As per emulator data |

## System Architecture Diagram

```mermaid
graph TB
    subgraph "Rover Emulators"
        E1[Rover 1 Emulator]
        E2[Rover 2 Emulator]
        E3[Rover 3 Emulator]
        E4[Rover 4 Emulator]
        E5[Rover 5 Emulator]
    end

    subgraph "Visualization Application"
        subgraph "Network Layer"
            NR[Network Receiver Thread]
            UDP1[UDP Socket: Pose 9001-9005]
            UDP2[UDP Socket: LiDAR 10001-10005]
            UDP3[UDP Socket: Telemetry 11001-11005]
            UDP4[UDP Socket: Commands 8001-8005]
        end

        subgraph "Data Processing"
            PP[Packet Parser]
            LR[LiDAR Reassembler]
            TS[Thread-Safe Buffers]
            DM[Data Manager]
        end

        subgraph "Rendering Thread"
            GL[OpenGL Context]
            SC[Scene Manager]
            CM[Camera Manager]
            TM[Terrain Mapper]
            RM[Rover Renderer]
            PC[Point Cloud Renderer]
        end

        subgraph "UI Layer"
            IM[ImGui Manager]
            CP[Control Panel]
            SP[Status Panel]
            VP[Viewport Panel]
            ST[Sci-Fi Theme]
        end
    end

    E1 -->|UDP Pose| UDP1
    E1 -->|UDP LiDAR| UDP2
    E1 -->|UDP Telemetry| UDP3
    E1 <-->|UDP Commands| UDP4

    E2 --> UDP1
    E2 --> UDP2
    E2 --> UDP3
    E2 <--> UDP4

    E3 --> UDP1
    E3 --> UDP2
    E3 --> UDP3
    E3 <--> UDP4

    E4 --> UDP1
    E4 --> UDP2
    E4 --> UDP3
    E4 <--> UDP4

    E5 --> UDP1
    E5 --> UDP2
    E5 --> UDP3
    E5 <--> UDP4

    NR --> UDP1
    NR --> UDP2
    NR --> UDP3
    NR --> UDP4

    NR --> PP
    PP --> LR
    LR --> TS
    TS --> DM

    DM --> SC
    SC --> CM
    SC --> TM
    SC --> RM
    SC --> PC

    GL --> SC
    IM --> CP
    IM --> SP
    IM --> VP
    ST --> IM

    CP --> DM
    SP --> DM
    VP --> CM
```

## Component Architecture

```mermaid
graph LR
    subgraph "Application Core"
        MAIN[Main Application]
        INIT[Initialization]
        LOOP[Main Loop]
    end

    subgraph "Threading Model"
        NET_THREAD[Network Thread]
        RENDER_THREAD[Render Thread]
    end

    subgraph "Network Components"
        SOCKET_MGR[Socket Manager]
        POSE_RECV[Pose Receiver]
        LIDAR_RECV[LiDAR Receiver]
        TELEM_RECV[Telemetry Receiver]
        CMD_SEND[Command Sender]
    end

    subgraph "Data Structures"
        ROVER_DATA[RoverData]
        POINT_CLOUD[PointCloud]
        TERRAIN_GRID[TerrainGrid]
        PACKET_QUEUE[PacketQueue]
    end

    subgraph "Rendering Components"
        SHADER_MGR[Shader Manager]
        MESH_BUILDER[Mesh Builder]
        CAMERA[Camera System]
        RENDERER[Renderer]
    end

    subgraph "UI Components"
        UI_MANAGER[UI Manager]
        THEME[Sci-Fi Theme]
        PANELS[UI Panels]
        CONTROLS[Control Widgets]
    end

    MAIN --> INIT
    INIT --> NET_THREAD
    INIT --> RENDER_THREAD
    MAIN --> LOOP

    NET_THREAD --> SOCKET_MGR
    SOCKET_MGR --> POSE_RECV
    SOCKET_MGR --> LIDAR_RECV
    SOCKET_MGR --> TELEM_RECV
    SOCKET_MGR --> CMD_SEND

    POSE_RECV --> PACKET_QUEUE
    LIDAR_RECV --> PACKET_QUEUE
    TELEM_RECV --> PACKET_QUEUE

    PACKET_QUEUE --> ROVER_DATA
    PACKET_QUEUE --> POINT_CLOUD
    POINT_CLOUD --> TERRAIN_GRID

    RENDER_THREAD --> SHADER_MGR
    RENDER_THREAD --> MESH_BUILDER
    RENDER_THREAD --> CAMERA
    RENDER_THREAD --> RENDERER

    ROVER_DATA --> RENDERER
    POINT_CLOUD --> RENDERER
    TERRAIN_GRID --> RENDERER

    RENDER_THREAD --> UI_MANAGER
    UI_MANAGER --> THEME
    UI_MANAGER --> PANELS
    PANELS --> CONTROLS
    CONTROLS --> CMD_SEND
```

## Data Flow

```mermaid
sequenceDiagram
    participant E as Emulator
    participant NR as Network Receiver
    participant DM as Data Manager
    participant RT as Render Thread
    participant UI as UI Layer

    E->>NR: UDP Pose Packet
    E->>NR: UDP LiDAR Chunks
    E->>NR: UDP Telemetry
    
    NR->>NR: Parse Packets
    NR->>NR: Reassemble LiDAR
    
    NR->>DM: Update Rover Data
    NR->>DM: Add Point Cloud
    NR->>DM: Update Telemetry
    
    DM->>RT: Latest Rover States
    DM->>RT: Point Cloud Buffer
    DM->>RT: Terrain Updates
    
    RT->>RT: Update Camera
    RT->>RT: Render Scene
    RT->>RT: Render UI
    
    UI->>DM: Request Command
    DM->>NR: Send Command
    NR->>E: UDP Command Packet
    
    E->>NR: Updated Telemetry
    NR->>DM: Button States
    DM->>UI: Update Display
```

## Threading Model

```mermaid
graph TD
    MAIN[Main Thread]
    
    subgraph "Network Thread"
        NET_LOOP[Network Loop]
        RECV[Receive UDP]
        PARSE[Parse Packets]
        BUFFER[Update Buffers]
    end
    
    subgraph "Render Thread"
        RENDER_LOOP[Render Loop]
        UPDATE[Update Scene]
        DRAW_3D[Draw 3D Scene]
        DRAW_UI[Draw UI]
        SWAP[Swap Buffers]
    end
    
    MAIN -->|Start| NET_LOOP
    MAIN -->|Start| RENDER_LOOP
    
    NET_LOOP --> RECV
    RECV --> PARSE
    PARSE --> BUFFER
    BUFFER -->|Mutex Lock| NET_LOOP
    
    RENDER_LOOP -->|Mutex Lock| UPDATE
    UPDATE --> DRAW_3D
    DRAW_3D --> DRAW_UI
    DRAW_UI --> SWAP
    SWAP --> RENDER_LOOP
```

## Module Structure

### 1. Network Module (`network/`)
- **UDPReceiver**: Manages UDP sockets for all rovers
- **PacketParser**: Parses binary UDP packets
- **LiDARReassembler**: Reassembles chunked LiDAR packets
- **CommandSender**: Sends button commands to rovers

### 2. Data Module (`data/`)
- **RoverData**: Stores pose, orientation, button states per rover
- **PointCloud**: Manages LiDAR point collections
- **TerrainGrid**: Grid-based terrain height map
- **DataManager**: Thread-safe data access and updates

### 3. Rendering Module (`render/`)
- **Renderer**: Main OpenGL rendering context
- **ShaderManager**: Shader compilation and management
- **Camera**: Camera system (free-fly, follow rover)
- **RoverRenderer**: Renders rover models
- **PointCloudRenderer**: Efficient point cloud rendering
- **TerrainRenderer**: Terrain mesh rendering

### 4. UI Module (`ui/`)
- **UIManager**: ImGui setup and management
- **SciFiTheme**: Custom sci-fi styling
- **ControlPanel**: Rover selection and control
- **StatusPanel**: Rover status display
- **ViewportPanel**: 3D viewport with controls

### 5. Core Module (`core/`)
- **Application**: Main application class
- **Timer**: Performance timing
- **Config**: Configuration management
- **Math**: Math utilities (using GLM)

## Sci-Fi UI Design Elements

### Visual Style
- **Color Scheme**: 
  - Primary: Cyan/Blue (#00FFFF, #00D4FF)
  - Secondary: Dark Blue/Black (#001122, #002244)
  - Accent: Orange/Amber (#FF8800, #FFAA00)
  - Background: Deep space black (#000011)
  - Text: Bright cyan/white (#CCFFFF)

### UI Components
- **Holographic panels**: Semi-transparent with glow effects
- **Scan lines**: Subtle animated scan line effects
- **Grid overlays**: Hexagonal or grid patterns
- **Glowing borders**: Neon-style borders on panels
- **Animated indicators**: Pulsing/breathing effects for active states
- **Terminal-style fonts**: Monospace fonts for data displays
- **Progress bars**: Animated sci-fi style progress indicators

### Layout
- **Left Panel**: Rover list and selection (holographic list)
- **Right Panel**: Selected rover status and controls (command center style)
- **Bottom Panel**: System status and metrics (HUD style)
- **3D Viewport**: Full-screen with overlay controls (minimal UI in viewport)

## Performance Considerations

### Optimization Strategies
1. **Point Cloud Rendering**: Use instanced rendering, VBOs for efficiency
2. **Terrain Updates**: Incremental mesh updates, only rebuild changed regions
3. **Threading**: Lock-free queues where possible, minimize mutex contention
4. **Memory Management**: Keep all points (no history limit), optimize storage
5. **Rendering**: Frustum culling, LOD for distant objects
6. **UI Rendering**: Batch ImGui draw calls, minimize UI updates

### Point Cloud Strategy
- Keep all LiDAR points (no pruning)
- Color by height gradient (low=blue, high=red)
- Efficient VBO management for growing point count
- Consider spatial data structures if needed

### Latency Targets
- Network receive: < 5ms
- Data processing: < 10ms
- Rendering: < 30ms
- Total: < 50ms end-to-end

## Dependencies

### Required Libraries
- **GLFW3**: Window and OpenGL context management
- **GLAD/GL3W**: OpenGL function loader
- **GLM**: Mathematics library
- **Dear ImGui**: Immediate-mode GUI
- **Standard C++17**: Threading, networking, etc.

### Build System
- **CMake**: Cross-platform build system
- **Compiler**: Clang (macOS) initially, GCC for Linux later
- **Target**: macOS first, Linux (Ubuntu 22.04) compatibility later

### Platform Notes
- Development on macOS with cross-platform code
- Use POSIX sockets (compatible with both platforms)
- OpenGL 3.3 Core Profile (works on both platforms)
- Test Linux compatibility before final delivery

## File Structure

```
visualization/
├── CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── core/
│   │   ├── Application.h/cpp
│   │   ├── Timer.h/cpp
│   │   └── Config.h/cpp
│   ├── network/
│   │   ├── UDPReceiver.h/cpp
│   │   ├── PacketParser.h/cpp
│   │   ├── LiDARReassembler.h/cpp
│   │   └── CommandSender.h/cpp
│   ├── data/
│   │   ├── RoverData.h/cpp
│   │   ├── PointCloud.h/cpp
│   │   ├── TerrainGrid.h/cpp
│   │   └── DataManager.h/cpp
│   ├── render/
│   │   ├── Renderer.h/cpp
│   │   ├── ShaderManager.h/cpp
│   │   ├── Camera.h/cpp
│   │   ├── RoverRenderer.h/cpp
│   │   ├── PointCloudRenderer.h/cpp
│   │   ├── TerrainRenderer.h/cpp
│   │   └── shaders/
│   │       ├── basic.vert
│   │       ├── basic.frag
│   │       ├── pointcloud.vert
│   │       ├── pointcloud.frag
│   │       └── terrain.vert/frag
│   └── ui/
│       ├── UIManager.h/cpp
│       ├── SciFiTheme.h/cpp
│       ├── ControlPanel.h/cpp
│       ├── StatusPanel.h/cpp
│       └── ViewportPanel.h/cpp
├── include/
│   └── common.h
└── README.md
```

