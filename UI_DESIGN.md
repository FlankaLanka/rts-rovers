# Sci-Fi UI Design Specification

## Design Philosophy

Create a futuristic, holographic command center interface that feels like a high-tech mission control system. The UI should be functional, visually striking, and maintain excellent readability for real-time monitoring.

## Key Design Decisions

| Feature | Implementation |
|---------|----------------|
| **Window** | 1280x720 default, resizable, fullscreen (F11) |
| **Rover Models** | Simple geometric shapes with unique colors |
| **Point Cloud** | Height-based gradient (blue→cyan→green→yellow→red) |
| **Terrain** | Toggleable wireframe/solid, height coloring |
| **Units** | Display in meters, scale-independent UI |
| **Offline Status** | Show at last position, dimmed, keep point cloud |

## Color Palette

### Primary Colors
```
Background:     #000011  (Deep space black)
Panel Dark:     #001122  (Dark blue-black)
Panel Medium:   #002244  (Medium dark blue)
Border:         #003366  (Dark blue border)

Primary Accent: #00FFFF  (Bright cyan)
Secondary:      #00D4FF  (Light cyan)
Tertiary:       #00AAFF  (Medium cyan)

Warning/Active: #FF8800  (Orange/amber)
Success:        #00FF88  (Green-cyan)
Error:          #FF4444  (Red)

Text Primary:   #CCFFFF  (Light cyan-white)
Text Secondary: #88CCFF  (Medium cyan)
Text Dim:       #4488AA  (Dim cyan)
```

### Usage Guidelines
- **Background**: Main window background
- **Panels**: Dark blue-black with subtle transparency
- **Borders**: Glowing cyan borders on active panels
- **Text**: Light cyan for primary text, dim cyan for secondary
- **Accents**: Orange for active/warning states, cyan for normal
- **Highlights**: Bright cyan for selections and important data

## Layout Structure

```
┌─────────────────────────────────────────────────────────────────┐
│  [Top Bar: System Status & Time]                                 │
├──────────┬──────────────────────────────────────────────┬─────────┤
│          │                                              │         │
│  LEFT    │          3D VIEWPORT                        │  RIGHT  │
│  PANEL   │          (Full Screen)                      │  PANEL  │
│          │          [Rover Models]                      │         │
│  Rover   │          [Point Clouds]                     │ Status  │
│  List    │          [Terrain Mesh]                     │ Panel   │
│          │                                              │         │
│  [1] R1  │          [Camera Controls Overlay]          │ Pos:    │
│  [2] R2  │          [Minimap?]                          │ X: 412  │
│  [3] R3  │                                              │ Y: 26   │
│  [4] R4  │                                              │ Z: 117  │
│  [5] R5  │                                              │         │
│          │                                              │ Orient: │
│  [Info]  │                                              │ Roll: 0 │
│          │                                              │ Pitch:0 │
│          │                                              │ Yaw: 0  │
│          │                                              │         │
│          │                                              │ Buttons:│
│          │                                              │ [0][1]  │
│          │                                              │ [2][3]  │
│          │                                              │         │
└──────────┴──────────────────────────────────────────────┴─────────┘
│  [Bottom Bar: Performance Metrics & Network Status]              │
└─────────────────────────────────────────────────────────────────┘
```

## UI Components

### 1. Left Panel - Rover Selection List

**Style**: Holographic list with glowing selection
```
┌─────────────────────┐
│  ROVER FLEET        │
├─────────────────────┤
│ ┌─────────────────┐ │
│ │ [●] ROVER 01    │ │ ← Selected (glowing)
│ │ Status: ONLINE  │ │
│ │ Points: 1,234   │ │
│ └─────────────────┘ │
│ ┌─────────────────┐ │
│ │ [○] ROVER 02    │ │
│ │ Status: ONLINE  │ │
│ │ Points: 2,456   │ │
│ └─────────────────┘ │
│ ┌─────────────────┐ │
│ │ [○] ROVER 03    │ │
│ │ Status: ONLINE  │ │
│ │ Points: 987     │ │
│ └─────────────────┘ │
└─────────────────────┘
```

**Features**:
- Each rover in a card/panel
- Glowing border when selected
- Status indicator (online/offline)
- Point cloud count
- Click to select
- Hover effects (subtle glow)

### 2. Right Panel - Status & Control

**Style**: Command center display
```
┌─────────────────────┐
│  ROVER 01 STATUS    │
├─────────────────────┤
│                     │
│  POSITION           │
│  X: 412.60          │
│  Y:  26.11          │
│  Z: 117.00          │
│                     │
│  ORIENTATION        │
│  Roll:   0.0°       │
│  Pitch:  0.0°       │
│  Yaw:    0.0°       │
│                     │
│  BUTTON STATES      │
│  ┌───┐ ┌───┐        │
│  │ 0 │ │ 1 │        │ ← Glowing when ON
│  └───┘ └───┘        │
│  ┌───┐ ┌───┐        │
│  │ 2 │ │ 3 │        │
│  └───┘ └───┘        │
│                     │
│  COMMAND            │
│  [Toggle Buttons]   │
│                     │
└─────────────────────┘
```

**Features**:
- Terminal-style data display
- Monospace font for numbers
- Glowing button indicators
- Control buttons for toggling
- Real-time updates

### 3. 3D Viewport

**Style**: Minimal overlay on 3D scene
```
┌─────────────────────────────────────┐
│ [Camera: Free] [Follow R1] [Reset] │ ← Top overlay
│                                     │
│                                     │
│         3D SCENE                   │
│         (Full Screen)               │
│                                     │
│                                     │
│                                     │
│ [FPS: 60] [Points: 5,234]          │ ← Bottom overlay
└─────────────────────────────────────┘
```

**Features**:
- Minimal UI overlay
- Camera mode indicator
- Quick controls
- Performance metrics
- Optional minimap in corner

### 4. Button Controls

**Style**: Sci-fi styled toggle buttons
```
Normal State:          Active State:
┌─────┐                ┌─────┐
│  0  │                │  0  │ ← Glowing border
└─────┘                └─────┘
  Dim                    Bright
```

**Design**:
- Square/rectangular buttons
- Glowing border when active
- Pulsing animation when toggled
- Hover effect (slight glow)
- Click feedback (brief flash)

### 5. Status Indicators

**Online/Offline Indicator**:
```
Online:  [●]  (Bright cyan, pulsing)
Offline: [○]  (Dim gray, static)
```

**Data Display**:
- Monospace font for numbers
- Color-coded values
- Smooth number transitions
- Units displayed clearly (meters, degrees)
- Scale-independent: UI element sizes remain constant regardless of camera zoom
- Position values: X, Y, Z in meters
- Orientation values: Roll, Pitch, Yaw in degrees

## Visual Effects

### 1. Glowing Borders
- Cyan glow on active panels
- Subtle pulsing animation
- Border width: 2-3px
- Glow intensity: Medium

### 2. Scan Lines (Optional)
- Subtle horizontal scan lines
- Slow vertical animation
- Low opacity (10-20%)
- Optional effect, can be toggled

### 3. Grid Overlay (Optional)
- Hexagonal or square grid
- Very subtle (5-10% opacity)
- Animated or static
- Optional background effect

### 4. Panel Transparency
- Panels: 80-90% opacity
- Background visible through panels
- Creates depth effect
- Maintains readability

### 5. Hover Effects
- Subtle glow on hover
- Smooth transitions
- Not distracting
- Clear feedback

## Typography

### Fonts
- **Primary**: Monospace font (e.g., 'Courier New', 'Consolas', 'Monaco')
- **UI Labels**: Sans-serif (system default or custom)
- **Numbers**: Monospace (for alignment)

### Sizes
- **Headers**: 18-20px
- **Body Text**: 14-16px
- **Data Values**: 16-18px (monospace)
- **Labels**: 12-14px

## Animation Guidelines

### Transitions
- **Duration**: 150-300ms
- **Easing**: Smooth ease-in-out
- **Purpose**: State changes, hover effects

### Pulsing
- **Duration**: 2-3 seconds
- **Intensity**: Subtle (10-20% opacity variation)
- **Usage**: Active states, important indicators

### Smooth Updates
- **Numbers**: Smooth transitions between values
- **Positions**: Smooth camera movements
- **States**: Smooth button state changes

## Implementation Notes

### ImGui Customization
- Use `ImGui::PushStyleColor()` for colors
- Custom draw functions for glowing borders
- Custom button rendering for sci-fi style
- Custom window decorations

### Shader Effects (Optional)
- Post-processing shader for scan lines
- Glow effect shader for borders
- Can be implemented later if needed

### Performance
- Keep animations lightweight
- Use ImGui's built-in animation system
- Avoid heavy shader effects initially
- Optimize for 60 FPS

## Example ImGui Style Configuration

```cpp
// Sci-Fi Theme Colors
ImVec4 bg_color = ImVec4(0.0f, 0.0f, 0.067f, 1.0f);      // #000011
ImVec4 panel_color = ImVec4(0.0f, 0.067f, 0.133f, 0.9f);  // #001122
ImVec4 border_color = ImVec4(0.0f, 0.2f, 0.4f, 1.0f);     // #003366
ImVec4 accent_color = ImVec4(0.0f, 1.0f, 1.0f, 1.0f);      // #00FFFF
ImVec4 text_color = ImVec4(0.8f, 1.0f, 1.0f, 1.0f);       // #CCFFFF
ImVec4 active_color = ImVec4(1.0f, 0.533f, 0.0f, 1.0f);   // #FF8800

// Apply to ImGui style
ImGuiStyle& style = ImGui::GetStyle();
style.Colors[ImGuiCol_WindowBg] = panel_color;
style.Colors[ImGuiCol_Border] = border_color;
style.Colors[ImGuiCol_Text] = text_color;
style.Colors[ImGuiCol_Button] = accent_color;
style.Colors[ImGuiCol_ButtonHovered] = active_color;
// ... more color settings
```

## Responsive Design

### Window Sizing
- Default: 1280x720
- Minimum: 1280x720
- Recommended: 1920x1080
- Support window resizing (drag borders)
- Fullscreen toggle: F11 key
- Maintain aspect ratios where needed

### Panel Sizing
- Left panel: 250-300px width
- Right panel: 300-350px width
- Viewport: Flexible, fills remaining space
- Panels can be collapsed/expanded

### Scale-Independent UI
- Unit labels maintain constant screen size regardless of zoom
- Position/orientation values always readable
- Camera distance doesn't affect UI text size

## Accessibility

### Readability
- High contrast for text
- Clear visual hierarchy
- Sufficient font sizes
- Color-blind friendly (don't rely solely on color)

### Usability
- Clear button labels
- Tooltips for controls
- Keyboard shortcuts
- Intuitive navigation

## Rendering Options Panel

### Terrain Display Options
```
┌─────────────────────┐
│  TERRAIN OPTIONS    │
├─────────────────────┤
│  [x] Show Terrain   │
│  [x] Solid Mode     │
│  [ ] Wireframe Mode │
│  [x] Height Colors  │
│                     │
│  POINT CLOUD        │
│  [x] Show Points    │
│  [x] Height Gradient│
│                     │
│  Total Points: 125K │
└─────────────────────┘
```

### Height Gradient Colors
- **Lowest**: Blue (#0066FF)
- **Low-Mid**: Cyan (#00FFFF)
- **Mid**: Green (#00FF66)
- **Mid-High**: Yellow (#FFFF00)
- **Highest**: Red (#FF4400)

Gradient automatically scales to min/max height in current point cloud.

## Rover Colors (5 Unique Colors)

| Rover | Color | Hex |
|-------|-------|-----|
| Rover 1 | Cyan | #00FFFF |
| Rover 2 | Magenta | #FF00FF |
| Rover 3 | Yellow | #FFFF00 |
| Rover 4 | Green | #00FF88 |
| Rover 5 | Orange | #FF8800 |

## Offline Rover Display

When a rover stops sending data (timeout ~2 seconds):
- Model becomes dimmed (50% opacity)
- Status shows "OFFLINE" in dim text
- Point cloud remains visible (not dimmed)
- Last known position/orientation displayed
- Rover remains selectable

## Future Enhancements (Optional)

- Animated background patterns
- Particle effects
- Sound effects (optional)
- Custom cursor
- Advanced shader effects
- Theme variations
- Detailed rover 3D models (to replace geometric shapes)

