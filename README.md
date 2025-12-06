# Terrafirma Rovers - Quick Start Guide

## Step 0: Clone repo and go to root directory of project

## Step 1: Install Dependencies

### macOS
```bash
brew install glfw glm
```

### Ubuntu/Debian
```bash
sudo apt-get install libglfw3-dev libglm-dev
```

## Step 2: Extract Data Files
```bash
make extract
```

## Step 3: Build the Emulator
```bash
make
```

## Step 4: Build the Visualization
```bash
cd visualization
mkdir build && cd build
cmake ..
make -j4
cd ../..
```

## Step 5: Run the Program

**Terminal 1 - Start the rover emulators:**
```bash
make run
```

**Terminal 2 - Start the visualization:**
```bash
cd visualization/build
./terrafirma_viz
```

The visualization window should open and display the 5 rovers with their LiDAR data.

## Controls

- **1-5**: Select rover
- **Right-click + mouse drag**: Rotate camera
- **WASD**: Move camera
- **F**: Follow selected rover
- **F11**: Toggle fullscreen
- **ESC**: Exit
