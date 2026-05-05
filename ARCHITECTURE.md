# Project Architecture

## Overview
A dual-mode simulation tool combining circuit AC analysis and EM wave medium editing in a single unified application.

## Core Structure

### Unified Data Model (`ed.h`)
- **`editor_t`**: Single unified structure for both modes
  - Circuit fields: `components`, `links`, `pins`, `head` (ground node)
  - EM fields: `shapes`, `active_material`, `draw_mode`, `poly_wip`
  - Shared: `grid` (pan/zoom), `resol`, `aspectratio`

### Mode System (`gui.h`)
- **`app_mode_t`**: `MODE_SELECT`, `MODE_CIRCUIT`, `MODE_EM`
- **`show_mode_select()`**: Startup splash screen with two mode buttons
- **`Dockspace()`**: Single dockspace host that conditionally renders:
  - Circuit panels: Simulation, Properties, Explorer, Diagram
  - EM panels: Material Palette, EM Canvas

### Circuit Mode (`gui.h`)
- Modified Nodal Analysis (MNA) solver using GiNaC
- Component library: R, L, C, sources, op-amps, coupled inductors
- Interactive schematic editor with drag-and-drop
- Bode plot generation for frequency response
- Keyboard shortcuts: R, C, L, V, I, G, T, O

### EM Mode (`em_editor.h`)
- **Shape drawing**: Rectangle, Circle, Polygon
- **Material properties**: σ (conductivity), εᵣ (permittivity), μᵣ (permeability)
- **Presets**: Copper, Aluminum, Iron, PTFE, FR4, Air, Water
- **Interaction**: Click-drag to draw, right-drag to pan, scroll to zoom
- **Selection**: Click shapes to edit properties, delete, or modify

## File Organization

```
main.cpp           - Entry point, GLFW/OpenGL setup, main loop
gui.h              - Mode selection, circuit UI, unified dockspace
em_editor.h        - EM canvas, material palette, shape drawing
ed.h               - Unified editor_t structure, EM types
components.h       - Circuit component definitions
matrix.h           - Linear algebra for circuit solving
```

## Design Principles

1. **Minimal duplication**: Single `editor_t` struct, single dockspace
2. **Mode isolation**: Circuit and EM code don't interfere
3. **Shared infrastructure**: Grid system, pan/zoom, ImGui docking
4. **Clean separation**: Each mode has its own panel functions

## Build System
- CMake-based build
- Dependencies: GLFW, OpenGL, ImGui, ImPlot, GiNaC, CLN
- Single binary: `build/sim`

## Current State
✅ Mode selection screen  
✅ Circuit AC analysis (existing)  
✅ EM medium editor with shape drawing  
✅ Material property editing  
✅ Unified dockspace architecture  
✅ Clean code structure  
✅ **FDTD wave simulation engine**  
✅ **Wave sources (point, plane)**  
✅ **Field probes (E-field, H-field)**  
✅ **Shape editing with resize handles**  
✅ **Real-time field visualization**  
✅ **Simulation controls (run/pause/step)**  

## Next Steps (Suggestions)

### High Priority
1. **PML Absorbing Boundaries**
   - Prevent edge reflections
   - Perfectly Matched Layer implementation
   - Configurable thickness

2. **File I/O**
   - Save/load circuit schematics
   - Save/load EM geometries
   - Export simulation results (CSV, images)
   - Export field animations

3. **Enhanced Visualization**
   - Vector field arrows (E and H)
   - Poynting vector display
   - Streamlines
   - Animation recording

### Medium Priority
4. **Frequency Analysis**
   - FFT of probe data
   - S-parameter extraction
   - Resonance detection
   - Bandwidth calculation

5. **Advanced Sources**
   - Gaussian pulse
   - Modulated signals
   - Custom waveforms
   - Multiple frequency sources

6. **Polygon Editing**
   - Vertex manipulation
   - Add/remove vertices
   - Bezier curves

7. **Circuit Enhancements**
   - Transient analysis (time-domain)
   - DC operating point
   - Component library expansion

### Low Priority
8. **UI Polish**
   - Undo/redo system
   - Grid snapping for EM shapes
   - Measurement tools (rulers, angles)
   - More keyboard shortcuts
   - Tooltips and help

9. **Performance**
   - GPU acceleration (CUDA/OpenCL)
   - Parallel CPU processing
   - Adaptive time stepping
   - Level-of-detail rendering

10. **3D Simulation**
    - Full 3D FDTD
    - 3D shape editor
    - Volume rendering
    - Far-field patterns

## Code Style Guidelines
- Use `inline` functions in headers
- Prefer `ImVec2` for 2D coordinates
- Use `editor_t*` for passing state
- Keep functions focused and small
- Comment complex algorithms
- Use descriptive variable names
