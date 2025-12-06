# Product Context

## Problem Statement
Construction and mining operations need real-time monitoring and control of autonomous vehicles. Operators must:
- Visualize vehicle positions and sensor data in 3D
- Understand terrain conditions from LiDAR scans
- Issue commands to control vehicle operations
- Switch between multiple vehicles seamlessly

## Solution Approach
A desktop application that:
1. **Receives** real-time telemetry from 5 rover emulators via UDP
2. **Visualizes** rovers, LiDAR point clouds, and terrain in 3D
3. **Maps** terrain dynamically from accumulated LiDAR points
4. **Provides** interactive UI for rover selection, monitoring, and control
5. **Supports** multiple control modes (autonomous, manual, RTS pathfinding, waypoint exploration)

## User Experience Goals
- **Intuitive**: Clear visual feedback, easy rover selection (1-5 keys)
- **Responsive**: Smooth camera movement, real-time updates
- **Informative**: Display position, orientation, button states, point counts
- **Flexible**: Multiple viewing modes (free camera, follow rover, 3rd person)
- **Modern**: Sci-fi themed UI with color-coded rovers and status indicators

## Key User Workflows
1. **Monitoring**: Select rover → View status → Observe movement
2. **Control**: Select rover → Press button → Send command → Monitor response
3. **Terrain Operations**: Select rover → Draw circle → Confirm → Watch operation
4. **Pathfinding**: Select rover → Enable RTS → Click terrain → Watch pathfinding
5. **Exploration**: Select rover → Enable WAY → Watch autonomous exploration

## Design Philosophy
- **Performance First**: Optimize for real-time updates, minimize latency
- **Visual Clarity**: Color coding, clear indicators, intuitive controls
- **Robustness**: Handle offline rovers, missing data, terrain gaps gracefully
- **Extensibility**: Modular design allows adding new control modes

