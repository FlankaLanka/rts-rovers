# Terrafirma Rovers - Implementation Task List

## Project Overview
Build a real-time 3D visualization and control system for 5 autonomous rovers with a sci-fi styled UI, using C++17, OpenGL, and ImGui.

## Design Decisions Summary

| Feature | Decision |
|---------|----------|
| **Platform** | macOS first, Linux later |
| **Rover Models** | Simple geometric shapes (different colors), models later |
| **Point Cloud Colors** | Height gradient (elevation-based) |
| **Terrain Rendering** | Toggle: wireframe/solid, both with height coloring |
| **Point History** | Keep all points (no limit) |
| **Offline Rovers** | Show at last position, keep point cloud |
| **Window** | 1280x720 default, resizable, fullscreen toggle |
| **Controls** | WASD + mouse, 1-5 for rover selection |
| **Units** | Display in UI, scale-independent sizing |

## Phase 1: Project Setup & Foundation

### 1.1 Project Structure Setup
- [ ] Create `visualization/` directory structure
- [ ] Set up CMake build system
- [ ] Create main CMakeLists.txt with C++17 standard
- [ ] Add dependency management (GLFW, GLM, ImGui)
- [ ] Create basic project structure (src/, include/, shaders/)
- [ ] Set up gitignore for build artifacts
- [ ] Create README.md for visualization project

### 1.2 Dependency Installation & Verification
- [ ] Install GLFW3 development libraries
- [ ] Install GLM library
- [ ] Set up ImGui (header-only or submodule)
- [ ] Set up GLAD/GL3W for OpenGL loading
- [ ] Verify all dependencies compile
- [ ] Test basic OpenGL context creation

### 1.3 Build System Configuration
- [ ] Configure CMake for macOS (Linux later)
- [ ] Add compiler flags (-Wall, -Wextra, -O2)
- [ ] Set up debug/release builds
- [ ] Add dependency finding (FindGLFW3, etc.)
- [ ] Test compilation of empty project
- [ ] Update main Makefile to build visualization

## Phase 2: Core Application Framework

### 2.1 Basic Application Structure
- [ ] Create Application class (singleton or main class)
- [ ] Implement window creation (GLFW)
- [ ] Set up OpenGL context (3.3+)
- [ ] Implement main loop structure
- [ ] Add basic error handling
- [ ] Implement cleanup and shutdown

### 2.2 Core Utilities
- [ ] Create Timer class for FPS and delta time
- [ ] Create Config class for settings
- [ ] Add logging system (simple console logging)
- [ ] Create math utilities wrapper (GLM integration)
- [ ] Add common headers (common.h)

### 2.3 Basic Window & Context
- [ ] Create window with default size 1280x720
- [ ] Enable window resizing
- [ ] Add fullscreen toggle (F11 key)
- [ ] Set up OpenGL viewport
- [ ] Implement basic input handling (keyboard, mouse)
- [ ] Add window resize callback
- [ ] Test basic rendering (clear screen with color)

## Phase 3: Network Layer

### 3.1 UDP Socket Infrastructure
- [ ] Create UDPReceiver class
- [ ] Implement socket creation for all rover ports
- [ ] Set up non-blocking UDP sockets
- [ ] Implement port binding for command sending
- [ ] Add error handling for socket operations
- [ ] Test basic UDP send/receive

### 3.2 Packet Structures
- [ ] Define PosePacket struct (match emulator)
- [ ] Define LidarPacketHeader struct
- [ ] Define LidarPoint struct
- [ ] Define LidarPacket struct
- [ ] Define VehicleTelem struct
- [ ] Add packet validation functions

### 3.3 Packet Parsing
- [ ] Create PacketParser class
- [ ] Implement PosePacket parsing
- [ ] Implement LidarPacket parsing
- [ ] Implement VehicleTelem parsing
- [ ] Add endianness handling
- [ ] Add packet validation and error handling

### 3.4 LiDAR Reassembly
- [ ] Create LiDARReassembler class
- [ ] Implement chunk collection by timestamp
- [ ] Handle out-of-order packet arrival
- [ ] Implement timeout for incomplete scans
- [ ] Add memory management (limit history)
- [ ] Test with sample packets

### 3.5 Command Sending
- [ ] Create CommandSender class
- [ ] Implement button command encoding (bitfield)
- [ ] Implement UDP command sending
- [ ] Add command queue/throttling
- [ ] Test command sending to emulator

### 3.6 Network Thread
- [ ] Create network receiver thread
- [ ] Implement thread-safe packet queues
- [ ] Add mutex protection for shared data
- [ ] Implement thread synchronization
- [ ] Add thread-safe shutdown
- [ ] Test multi-threaded packet reception

## Phase 4: Data Management

### 4.1 Rover Data Structure
- [ ] Create RoverData class
- [ ] Store position (x, y, z)
- [ ] Store orientation (roll, pitch, yaw)
- [ ] Store button states
- [ ] Store timestamp
- [ ] Add thread-safe getters/setters

### 4.2 Point Cloud Management
- [ ] Create PointCloud class
- [ ] Implement point storage (vector of points)
- [ ] Add point cloud per rover
- [ ] Keep all points (no history limit)
- [ ] Add efficient point addition (batch append)
- [ ] Store min/max height for gradient coloring
- [ ] Implement point cloud clearing (manual only)

### 4.3 Terrain Grid
- [ ] Create TerrainGrid class
- [ ] Implement grid-based height map
- [ ] Define grid resolution (e.g., 1m cells)
- [ ] Implement point-to-grid mapping
- [ ] Add height accumulation (max/min/average)
- [ ] Implement dirty region tracking

### 4.4 Data Manager
- [ ] Create DataManager class (thread-safe)
- [ ] Implement rover data storage (5 rovers)
- [ ] Add point cloud management per rover
- [ ] Integrate terrain grid updates
- [ ] Implement thread-safe data access
- [ ] Add data update callbacks/notifications

## Phase 5: Rendering Foundation

### 5.1 Shader System
- [ ] Create ShaderManager class
- [ ] Implement shader loading from files
- [ ] Implement shader compilation
- [ ] Add shader error checking
- [ ] Create basic vertex shader
- [ ] Create basic fragment shader
- [ ] Test shader compilation

### 5.2 Basic Rendering
- [ ] Create Renderer class
- [ ] Implement OpenGL state management
- [ ] Set up basic rendering pipeline
- [ ] Implement clear and swap buffers
- [ ] Add depth testing
- [ ] Test rendering a simple triangle

### 5.3 Camera System
- [ ] Create Camera class
- [ ] Implement free-fly camera
- [ ] Add mouse look controls
- [ ] Add keyboard movement (WASD)
- [ ] Implement camera following rover
- [ ] Add smooth camera transitions
- [ ] Implement view and projection matrices

### 5.4 Coordinate System
- [ ] Define world coordinate system
- [ ] Implement coordinate transformations
- [ ] Add axis visualization (debug)
- [ ] Test coordinate system alignment

## Phase 6: 3D Scene Rendering

### 6.1 Rover Rendering
- [ ] Create RoverRenderer class
- [ ] Design simple geometric shapes (box/pyramid for orientation)
- [ ] Create rover mesh/geometry (simple shapes)
- [ ] Implement per-rover unique colors (5 distinct colors)
- [ ] Implement rover positioning
- [ ] Add rover orientation rendering (arrow/cone for direction)
- [ ] Add rover label/ID display
- [ ] Show offline rovers at last known position (dimmed)
- [ ] Plan for future model replacement

### 6.2 Point Cloud Rendering
- [ ] Create PointCloudRenderer class
- [ ] Implement point cloud VBO creation
- [ ] Use instanced rendering for efficiency
- [ ] Add point size control
- [ ] Implement height-based gradient coloring (blue=low, red=high)
- [ ] Dynamic color range based on min/max height
- [ ] Optimize for large point counts (keep all points)
- [ ] Efficient VBO updates for growing point cloud

### 6.3 Terrain Rendering
- [ ] Create TerrainRenderer class
- [ ] Implement grid-to-mesh conversion
- [ ] Create terrain mesh from height map
- [ ] Implement incremental mesh updates
- [ ] Add terrain shading (height-based colors)
- [ ] Implement terrain normals calculation
- [ ] Add wireframe rendering mode (toggleable)
- [ ] Add solid rendering mode (toggleable)
- [ ] Height-based coloring for both modes
- [ ] UI toggle for wireframe/solid/both

### 6.4 Scene Management
- [ ] Create SceneManager class
- [ ] Integrate all renderers
- [ ] Implement scene update loop
- [ ] Add render ordering
- [ ] Implement frustum culling
- [ ] Add scene statistics (FPS, point count, etc.)

## Phase 7: Sci-Fi UI Implementation

### 7.1 ImGui Setup
- [ ] Integrate ImGui with OpenGL
- [ ] Set up ImGui rendering
- [ ] Create UIManager class
- [ ] Implement UI update loop
- [ ] Add ImGui input handling

### 7.2 Sci-Fi Theme
- [ ] Create SciFiTheme class
- [ ] Define color palette (cyan, dark blue, orange)
- [ ] Implement custom ImGui style
- [ ] Add glowing border effects
- [ ] Create holographic panel style
- [ ] Add scan line effects (optional shader)
- [ ] Implement animated UI elements

### 7.3 Control Panel
- [ ] Create ControlPanel class
- [ ] Design rover selection list
- [ ] Implement rover selection logic
- [ ] Add button controls (4 buttons per rover)
- [ ] Create button state indicators
- [ ] Add visual feedback for selection
- [ ] Implement rover switching

### 7.4 Status Panel
- [ ] Create StatusPanel class
- [ ] Display selected rover position (with units: meters)
- [ ] Display rover orientation (degrees)
- [ ] Show button states (visual indicators)
- [ ] Add timestamp display
- [ ] Create terminal-style data display
- [ ] Add status indicators (online/offline)
- [ ] Units display scale-independent (fixed screen size regardless of zoom)
- [ ] Show "OFFLINE" status for inactive rovers

### 7.5 Viewport Panel
- [ ] Create ViewportPanel class
- [ ] Integrate 3D viewport with ImGui
- [ ] Add viewport controls overlay
- [ ] Implement camera mode toggle (free-fly / follow)
- [ ] Add rover follow button
- [ ] Create minimap (optional)
- [ ] Add rendering options panel:
  - [ ] Terrain: wireframe toggle
  - [ ] Terrain: solid toggle
  - [ ] Height coloring toggle
  - [ ] Point cloud visibility toggle
- [ ] Fullscreen toggle (F11)

### 7.6 System Status Panel
- [ ] Create system metrics display
- [ ] Show FPS counter
- [ ] Display network statistics
- [ ] Show point cloud counts
- [ ] Add terrain statistics
- [ ] Create performance graphs (optional)

## Phase 8: Integration & Features

### 8.1 Thread Integration
- [ ] Integrate network thread with render thread
- [ ] Implement data synchronization
- [ ] Add thread-safe data access patterns
- [ ] Test concurrent operations
- [ ] Optimize mutex usage

### 8.2 Real-time Updates
- [ ] Connect network data to renderer
- [ ] Implement real-time rover updates
- [ ] Add real-time point cloud updates
- [ ] Implement real-time terrain updates
- [ ] Test with single rover first
- [ ] Test with all 5 rovers

### 8.3 Camera Controls
- [ ] Implement free-fly camera controls
- [ ] Add rover follow mode
- [ ] Implement smooth camera transitions
- [ ] Add camera reset function
- [ ] Create camera presets (top-down, etc.)

### 8.4 Command System
- [ ] Connect UI buttons to command sender
- [ ] Implement button state synchronization
- [ ] Add command feedback (visual/audio)
- [ ] Test command sending
- [ ] Verify telemetry updates

## Phase 9: Optimization & Polish

### 9.1 Performance Optimization
- [ ] Profile application performance
- [ ] Optimize point cloud rendering
- [ ] Optimize terrain mesh updates
- [ ] Reduce mutex contention
- [ ] Optimize UI rendering
- [ ] Add performance metrics

### 9.2 Memory Management
- [ ] Keep all points (no pruning) - optimize storage
- [ ] Add memory pool for frequent allocations
- [ ] Optimize terrain grid memory usage
- [ ] Add memory leak detection
- [ ] Profile memory usage
- [ ] Monitor total point count in UI

### 9.3 Rendering Optimization
- [ ] Implement frustum culling
- [ ] Add LOD for distant objects
- [ ] Optimize shader usage
- [ ] Reduce draw calls
- [ ] Implement occlusion culling (optional)

### 9.4 UI Polish
- [ ] Refine sci-fi theme colors
- [ ] Add smooth animations
- [ ] Improve button feedback
- [ ] Add tooltips
- [ ] Polish layout and spacing
- [ ] Add keyboard shortcuts

### 9.5 Error Handling
- [ ] Add comprehensive error handling
- [ ] Implement graceful degradation
- [ ] Add network error recovery
- [ ] Handle missing rovers gracefully
- [ ] Add user-friendly error messages

## Phase 10: Testing & Validation

### 10.1 Unit Testing
- [ ] Test packet parsing
- [ ] Test LiDAR reassembly
- [ ] Test data structures
- [ ] Test math utilities
- [ ] Test terrain grid updates

### 10.2 Integration Testing
- [ ] Test with single rover
- [ ] Test with all 5 rovers
- [ ] Test with --no-noise flag
- [ ] Test with noise enabled
- [ ] Test command sending
- [ ] Test camera controls

### 10.3 Performance Testing
- [ ] Measure latency (target < 50ms)
- [ ] Test on target hardware (ThinkPad T16)
- [ ] Profile memory usage
- [ ] Test with maximum data load
- [ ] Verify smooth 60 FPS

### 10.4 User Testing
- [ ] Test UI intuitiveness
- [ ] Verify rover switching works
- [ ] Test all camera modes
- [ ] Verify button controls
- [ ] Test on macOS first
- [ ] Test on Ubuntu 22.04 (before delivery)

## Phase 11: Documentation & Delivery

### 11.1 Code Documentation
- [ ] Add code comments
- [ ] Document public APIs
- [ ] Create architecture documentation
- [ ] Document build process
- [ ] Document dependencies

### 11.2 User Documentation
- [ ] Create user guide
- [ ] Document UI controls
- [ ] Add keyboard shortcuts reference
- [ ] Create troubleshooting guide
- [ ] Add screenshots/demo

### 11.3 Build Instructions
- [ ] Document dependency installation
- [ ] Create build instructions
- [ ] Add CMake configuration guide
- [ ] Test build on clean system
- [ ] Create installation script (optional)

### 11.4 Final Polish
- [ ] Code cleanup and formatting
- [ ] Remove debug code
- [ ] Optimize final build
- [ ] Create release package
- [ ] Prepare submission materials

## Sci-Fi UI Specific Tasks

### Visual Design
- [ ] Design color scheme (cyan/blue/orange)
- [ ] Create panel layouts
- [ ] Design button styles
- [ ] Create iconography (if needed)
- [ ] Design status indicators

### Theme Implementation
- [ ] Implement dark background
- [ ] Add glowing borders
- [ ] Create holographic effects
- [ ] Add scan line animations
- [ ] Implement grid overlays
- [ ] Create terminal-style fonts

### UI Components
- [ ] Holographic rover list
- [ ] Command center control panel
- [ ] HUD-style status display
- [ ] Sci-fi styled buttons
- [ ] Animated progress indicators
- [ ] Glowing selection highlights

## Estimated Timeline

- **Phase 1-2**: 1-2 days (Setup & Foundation)
- **Phase 3**: 2-3 days (Network Layer)
- **Phase 4**: 1-2 days (Data Management)
- **Phase 5-6**: 3-4 days (Rendering)
- **Phase 7**: 2-3 days (UI Implementation)
- **Phase 8**: 2 days (Integration)
- **Phase 9**: 2-3 days (Optimization)
- **Phase 10**: 1-2 days (Testing)
- **Phase 11**: 1 day (Documentation)

**Total Estimated Time**: 15-22 days

## Priority Order

1. **Critical Path**: Phases 1-8 (Core functionality)
2. **Important**: Phase 9 (Performance)
3. **Nice to Have**: Phase 10-11 (Polish & Documentation)

## Notes

- Start with single rover, then scale to 5
- Test incrementally after each phase
- Keep sci-fi theme consistent throughout
- Focus on sub-50ms latency requirement
- Ensure lightweight for standard hardware
- **Platform**: Develop on macOS, test Linux before delivery
- **Rover Models**: Simple shapes now, detailed models later
- **Point Cloud**: Keep all points, color by height
- **Terrain**: Toggleable wireframe/solid with height coloring
- **Offline Handling**: Show at last position, keep data visible

