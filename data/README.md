# Rover Data Files

This directory contains telemetry data for 5 autonomous rovers used by the emulator.

## Data Files

### Extracted Data (`.dat` files)
- `rover1.dat` - 115 MB, 3,835 time steps
- `rover2.dat` - 273 MB, 3,835 time steps  
- `rover3.dat` - 110 MB, 3,835 time steps
- `rover4.dat` - 111 MB, 3,835 time steps
- `rover5.dat` - 106 MB, 3,835 time steps

**Total:** ~715 MB of extracted data, 19,175 total time steps across all rovers

### Archive Files (`.tar.xz` files)
- `rover1.dat.tar.xz` - 35 MB (compressed)
- `rover2.dat.tar.xz` - 83 MB (compressed)
- `rover3.dat.tar.xz` - 35 MB (compressed)
- `rover4.dat.tar.xz` - 34 MB (compressed)
- `rover5.dat.tar.xz` - 35 MB (compressed)

**Note:** Archive files are kept as backups. The extracted `.dat` files are used by the emulator.

## Data Format

Each `.dat` file contains one line per time step. The format is:

```
posX,posY,posZ,rotX,rotY,rotZ; x1,y1,z1; x2,y2,z2; x3,y3,z3; ...
```

Where:
- **Pose data** (before first `;`): 6 comma-separated floats
  - `posX, posY, posZ`: Position in 3D space
  - `rotX, rotY, rotZ`: Rotation angles in degrees (roll, pitch, yaw)
- **LiDAR points** (after first `;`): Multiple `x,y,z` triplets separated by `;`
  - Each point represents a 3D coordinate from the LiDAR sensor

## Data Characteristics

- **Time steps per rover:** 3,835
- **Sampling rate:** 10 Hz (100ms intervals)
- **Duration per rover:** ~383.5 seconds (~6.4 minutes)
- **Coordinate system:** 3D Cartesian (X, Y, Z)
- **Units:** Appears to be meters (verify with actual data)

## Usage

The rover emulator reads these files line-by-line and streams the data over UDP at 10 Hz.

To extract the data files from archives:
```bash
make extract
```

The emulator will automatically use the extracted `.dat` files when running.

