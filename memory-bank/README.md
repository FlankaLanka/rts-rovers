# Memory Bank

This directory contains comprehensive documentation for the Terrafirma Rovers project. The memory bank serves as a knowledge repository that persists across sessions.

## File Structure

### Core Documents
- **`projectbrief.md`**: Foundation document - project overview, objectives, requirements
- **`productContext.md`**: Why the project exists, user experience goals, workflows
- **`systemPatterns.md`**: System architecture, design patterns, component relationships
- **`techContext.md`**: Technology stack, build setup, network protocol, file structure
- **`activeContext.md`**: Current work focus, recent changes, active features
- **`progress.md`**: Completed features, known issues, testing status

## How to Use

### For New Contributors
1. Start with `projectbrief.md` for high-level understanding
2. Read `productContext.md` to understand user needs
3. Review `systemPatterns.md` for architecture overview
4. Check `techContext.md` for setup instructions
5. See `progress.md` for what's done and what's left

### For Development
- **`activeContext.md`**: Current state, recent changes, active work
- **`systemPatterns.md`**: Design patterns, component interactions
- **`techContext.md`**: Technical details, APIs, protocols

### For Planning
- **`progress.md`**: Feature completion status
- **`productContext.md`**: User workflows, requirements
- **`projectbrief.md`**: Core objectives, success criteria

## Maintenance

The memory bank should be updated when:
- Major features are added or changed
- Architecture decisions are made
- New patterns emerge
- Project status changes significantly

## Quick Reference

### Key Numbers
- **5 Rovers**: Unique IDs 1-5
- **4 UDP Ports per Rover**: Pose (9000+), LiDAR (10000+), Telemetry (11000+), Commands (8000+)
- **10Hz Update Rate**: From emulator
- **Target FPS**: 60+ FPS
- **Coordinate System**: Y-up (Y = height)

### Key Files
- **`visualization/include/common.h`**: Packet structures, rover state
- **`visualization/src/core/Application.cpp`**: Main loop, event handling
- **`visualization/src/data/DataManager.cpp`**: Central data hub
- **`visualization/src/pathfinding/AStar.cpp`**: Pathfinding algorithm

### Control Modes
- **Button 0**: Engine ON/OFF
- **Button 1**: DIG operation
- **Button 2**: PILE operation
- **Button 3**: Manual control (WASD)
- **Button 4**: RTS pathfinding
- **Button 5**: WAY waypoint exploration

