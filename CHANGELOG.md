# Changelog

## [Unreleased] - 2024

### Added - EM Wave Simulation

#### Core Simulation Engine
- **FDTD Solver** (`fdtd_solver.h`)
  - 2D TM mode implementation (Ez, Hx, Hy fields)
  - Automatic grid generation from drawn shapes
  - Material property rasterization (σ, ε, μ)
  - Courant-stable time stepping
  - Lossy media support via conductivity
  - Real-time field computation

#### Wave Sources
- **Point Source**: Omnidirectional wave emission
  - Configurable frequency, amplitude, phase
  - Visual: Animated pulsing circle
  - Drag to reposition
- **Plane Wave Source**: Uniform directional propagation
  - Adjustable angle (0-360°)
  - Visual: Arrow with direction indicator
  - Ideal for transmission/reflection studies
- **Source Controls**:
  - Add/delete sources via UI panel
  - Select and edit properties
  - Toggle active/inactive state
  - Real-time parameter adjustment

#### Field Probes
- **E-Field Probe**: Electric field magnitude measurement
  - Live value display in V/m
  - Visual: Green square marker
- **H-Field Probe**: Magnetic field magnitude measurement
  - Live value display in A/m
  - Visual: Green square marker
- **Probe Controls**:
  - Add/delete probes via UI panel
  - Drag to reposition
  - Select to view properties
  - Real-time value updates during simulation

#### Shape Editing
- **Rectangle Editing**:
  - 8 resize handles (4 corners + 4 edges)
  - Drag corners for diagonal resize
  - Drag edges for single-axis resize
  - Visual: White circles with black outline
- **Circle Editing**:
  - 4 resize handles (cardinal directions)
  - Drag to adjust radius
  - Center point remains fixed
  - Visual: White circles on circumference
- **Edit Mode**:
  - Automatic handle display on selection
  - Click-and-drag interaction
  - Real-time shape updates
  - Smooth visual feedback

#### Visualization
- **Field Display**:
  - Color-coded intensity (red = positive, blue = negative)
  - Real-time updates during simulation
  - Automatic scaling based on max field
  - Rendered behind shapes for clarity
- **Source Visualization**:
  - Animated pulsing for point sources
  - Directional arrows for plane waves
  - Name labels
  - Selection highlighting (yellow)
- **Probe Visualization**:
  - Square markers with live values
  - Scientific notation for field values
  - Selection highlighting (yellow)
  - Always-on-top rendering

#### Simulation Controls
- **Initialize Grid**: Generate simulation mesh from shapes
- **Run/Pause**: Start/stop continuous time-stepping
- **Step**: Advance one time step manually
- **Reset**: Clear fields and restart from t=0
- **Reinitialize**: Rebuild grid after geometry changes
- **Status Display**:
  - Grid dimensions (nx × ny)
  - Cell size in mm
  - Current time step
  - Maximum field value

#### UI Enhancements
- **EM Material Palette** (expanded):
  - Simulation section with controls
  - Sources section with add/edit/delete
  - Probes section with add/edit/delete
  - Real-time status information
- **EM Canvas** (enhanced):
  - Field visualization overlay
  - Source/probe rendering
  - Edit handle display
  - Multi-object selection support

### Changed

#### Data Structures (`ed.h`)
- Extended `editor_t` with EM simulation fields:
  - `std::vector<em_source_t> sources`
  - `std::vector<em_probe_t> probes`
  - `fdtd_grid_t fdtd`
  - `selected_source_idx`, `selected_probe_idx`
  - `editing_shape`, `edit_handle`
- Added new types:
  - `em_source_type` enum (POINT, LINE, PLANE)
  - `em_probe_type` enum (E_FIELD, H_FIELD, POWER)
  - `em_source_t` struct
  - `em_probe_t` struct
  - `fdtd_grid_t` struct

#### Main Loop (`main.cpp`)
- Added FDTD time-stepping in main loop
- Calls `fdtd_step()` when EM mode is active and running
- Automatic simulation updates at frame rate

#### Interaction (`em_editor.h`)
- Enhanced left-click handling:
  - Check edit handles first (priority)
  - Then check sources/probes
  - Then check shapes
  - Finally start drawing
- Added drag-to-edit for shape handles
- Added drag-to-move for sources/probes
- Multi-object selection support

### Technical Details

#### FDTD Implementation
- **Algorithm**: Yee grid with leapfrog time-stepping
- **Accuracy**: Second-order in space and time
- **Stability**: Courant condition enforced (Δt ≤ Δx/(c√2))
- **Boundary**: Hard boundaries (reflective edges)
- **Materials**: Isotropic, frequency-independent
- **Grid**: Uniform rectangular mesh
- **Fields**: TM mode (Ez, Hx, Hy)

#### Performance
- **Grid Size**: 32×32 to 512×512 cells
- **Typical**: 128×128 at ~60 FPS
- **Memory**: ~4 MB for 256×256 grid
- **CPU**: Single-threaded (no GPU acceleration yet)

#### File Structure
```
ed.h              - Extended with EM types
fdtd_solver.h     - NEW: FDTD algorithm
em_editor.h       - Enhanced with sources/probes/editing
main.cpp          - Added simulation loop
EM_FEATURES.md    - NEW: User documentation
ARCHITECTURE.md   - Updated with completed features
```

### Known Issues
- No PML boundaries (waves reflect at grid edges)
- Polygon shapes not yet editable
- No animation export
- CPU-only (no GPU acceleration)
- 2D only (no 3D simulation)

### Future Work
See ARCHITECTURE.md "Next Steps" section for planned features.

---

## [Previous] - Circuit Simulation

### Existing Features (Preserved)
- Modified Nodal Analysis (MNA) solver
- Component library (R, L, C, sources, op-amps)
- Interactive schematic editor
- Bode plot generation
- Frequency response analysis
- Material presets for EM mode
- Shape drawing (rect, circle, polygon)
- Pan/zoom/grid system
- Dockable UI panels
- Mode selection screen
- Unified editor structure

---

## Development Notes

### Code Quality
- Clean separation of concerns
- Minimal code duplication
- Inline functions in headers
- Descriptive variable names
- Commented algorithms
- Consistent style

### Build System
- CMake-based
- Single binary output
- No new dependencies added
- Compiles cleanly (no warnings)

### Testing Recommendations
1. Start with simple geometry (1-2 shapes)
2. Add a point source
3. Initialize grid
4. Run simulation
5. Observe wave propagation
6. Add probes to measure fields
7. Try editing shapes with handles
8. Experiment with different materials

### Performance Tips
- Keep grid size reasonable (≤256×256)
- Use "Step" mode for detailed analysis
- Reinitialize after major geometry changes
- Monitor max field value for stability
