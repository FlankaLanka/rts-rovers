# Active Context

## Current State
The project is **feature-complete** with all core functionality implemented and working. Recent additions include RTS pathfinding and WAY waypoint modes.

## Recent Changes (Latest Session)
1. **A* Pathfinding**: Implemented grid-based pathfinding on terrain
2. **RTS Mode**: Click-to-move with path visualization
3. **WAY Mode**: Auto-spawning waypoints for exploration
4. **Path Rendering**: Visual path lines and destination markers
5. **Model Loading**: Support for OBJ, GLTF, and GLB formats
6. **Time Synchronization**: Fixed offline/online status bug with shared TimeUtil

## Active Features

### Rover Control Modes
1. **Normal Mode**: Default UDP-driven movement
2. **Button 0 (Engine)**: Toggle engine ON/OFF (pauses UDP data reading)
3. **Button 1 (DIG)**: Draw circle → Dig terrain (25m max depth, 5m steps)
4. **Button 2 (PILE)**: Draw circle → Pile terrain (25m max height, 5m steps)
5. **Button 3 (CTRL)**: Manual control with WASD (3rd person camera)
6. **Button 4 (RTS)**: Click terrain to pathfind (A* algorithm)
7. **Button 5 (WAY)**: Auto-explore with waypoints (20-50m range)

### Camera Modes
- **Free Camera**: WASD movement, right-click drag to rotate
- **Follow Rover**: Camera follows selected rover
- **3rd Person**: Used in manual control mode (behind rover, facing forward)

### UI Features
- **Fleet Panel**: List of all 5 rovers with status indicators
- **Status Panel**: Selected rover details, button controls
- **Settings Panel**: Toggle point clouds, terrain, wireframe
- **Operation Panel**: Dig/pile operation status and controls

## Current Work Focus
- **Stable**: All features implemented and tested
- **Ready for Demo**: System is fully functional
- **No Active Bugs**: Known issues resolved

## Next Steps (If Needed)
1. Performance optimization (if FPS drops with large point clouds)
2. Additional rover models (if needed)
3. Enhanced pathfinding (if obstacles need better handling)
4. UI polish (if requested)

## Key Files Modified Recently
- `visualization/src/pathfinding/`: New A* and path rendering
- `visualization/src/core/Application.cpp`: RTS/WAY mode integration
- `visualization/src/ui/UIManager.cpp`: Button 4 and 5 UI
- `visualization/include/TimeUtil.h`: Shared time source fix

## Important Notes
- **Rover Control Flags**: `m_roverControlled` prevents UDP updates during operations
- **Path State**: Each rover has independent path state (`m_currentPath`, `m_pathIndex`)
- **Mode Exclusivity**: Only one control mode active per rover (manual/RTS/WAY)
- **Terrain Operations**: Independent from pathfinding (can't run simultaneously)

