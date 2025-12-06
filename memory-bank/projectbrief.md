# Project Brief: Terrafirma Rovers

## Overview
A real-time 3D visualization and command system for multiple autonomous construction rovers (excavators, bulldozers, roller-compactors) operating in a terrain environment. The system visualizes rover positions, LiDAR sensor data, and provides interactive control interfaces.

## Core Objectives
1. **Real-time 3D Visualization**: Render 5 rovers and their LiDAR point clouds in a 3D environment
2. **Dynamic Terrain Mapping**: Build and update terrain mesh from incoming LiDAR measurements
3. **Rover Control Interface**: Monitor and command rovers via button controls (4 buttons per rover)
4. **Performance**: Sub-50ms end-to-end latency on standard hardware (Ubuntu 22.04, ThinkPad T16 Gen 3)
5. **Network Communication**: All data exchange via UDP on localhost

## Key Requirements
- **5 Rovers**: Each with unique ID, position, orientation, and LiDAR stream
- **UDP Communication**: 
  - Pose data: ports 9001-9005
  - LiDAR data: ports 10001-10005
  - Button telemetry: ports 11001-11005
  - Button commands: ports 8001-8005
- **Coordinate System**: Y-up (Y = height, X/Z = horizontal plane)
- **Update Rate**: 10Hz from emulator
- **Sci-Fi UI Theme**: Modern, futuristic interface design

## Success Criteria
- Real-time visualization with smooth updates
- Intuitive rover selection and control
- Accurate terrain mapping from LiDAR
- Responsive UI with minimal lag
- Support for multiple rover control modes (manual, RTS, waypoint)

## Project Structure
```
terrafirma-rovers/
├── emulator/          # Rover emulator (reads .dat files, sends UDP)
├── data/              # Rover telemetry data files (.dat)
├── visualization/     # Main application (C++/OpenGL)
└── memory-bank/       # Project documentation
```

## Technology Stack
- **Language**: C++17/20
- **Graphics**: OpenGL 3.3+ (via GLFW)
- **UI**: Dear ImGui
- **Math**: GLM
- **Networking**: UDP sockets
- **Build**: CMake

