# EM Wave Simulation Features

## Overview
The EM mode now includes a complete FDTD (Finite-Difference Time-Domain) solver for simulating electromagnetic wave propagation through custom-drawn media.

## New Features

### 1. FDTD Simulation Engine
- **2D TM Mode**: Simulates Ez, Hx, Hy field components
- **Automatic Grid Generation**: Converts drawn shapes into simulation mesh
- **Material Rasterization**: Maps material properties (σ, ε, μ) onto grid cells
- **Courant Stability**: Automatically calculates stable time step
- **Real-time Visualization**: Color-coded field intensity display
  - Red = positive E-field
  - Blue = negative E-field
  - Intensity = field magnitude

### 2. Wave Sources
Three types of electromagnetic sources:

#### Point Source
- Omnidirectional wave emission
- Configurable frequency, amplitude, phase
- Visual: Pulsing red circle
- Use case: Antenna simulation, point emitters

#### Plane Wave
- Uniform wave propagation in specified direction
- Adjustable angle (0-360°)
- Visual: Arrow showing propagation direction
- Use case: Incident wave testing, transmission studies

#### Line Source
- Linear wave emission
- Visual: Line with wave indicators
- Use case: Waveguide excitation

**Source Controls:**
- Add via "Sources" panel
- Click to select and edit properties
- Drag to reposition
- Toggle active/inactive
- Delete unwanted sources

### 3. Field Probes
Real-time field measurement at specific points:

#### E-Field Probe
- Measures electric field magnitude (Ez component)
- Display: Green square with live value
- Units: V/m

#### H-Field Probe
- Measures magnetic field magnitude (√(Hx² + Hy²))
- Display: Green square with live value
- Units: A/m

#### Power Probe (Future)
- Will measure Poynting vector (power flow)

**Probe Controls:**
- Add via "Probes" panel
- Click to select
- Drag to reposition
- Live value updates during simulation
- Delete unwanted probes

### 4. Shape Editing
Interactive resize and reshape capabilities:

#### Rectangle Editing
- **8 handles**: 4 corners + 4 edge midpoints
- Drag corners to resize diagonally
- Drag edges to resize in one direction
- Visual: White circles with black outline

#### Circle Editing
- **4 handles**: Cardinal directions (N, S, E, W)
- Drag any handle to change radius
- Center remains fixed
- Visual: White circles on circumference

#### Polygon Editing (Future)
- Vertex manipulation
- Add/remove vertices
- Smooth/sharpen corners

**How to Edit:**
1. Select a shape (click on it)
2. Handles appear automatically
3. Click and drag any handle
4. Release to apply changes

### 5. Simulation Controls

#### Initialize Grid
- Analyzes all shapes to determine simulation bounds
- Creates uniform grid (default: 1mm cells)
- Rasterizes material properties
- Allocates field arrays
- Status: Shows grid dimensions (e.g., "128x128 cells")

#### Run/Pause/Step
- **Run**: Continuous time-stepping (real-time animation)
- **Pause**: Freeze simulation at current state
- **Step**: Advance one time step manually
- **Reset**: Clear all fields, restart from t=0

#### Reinitialize
- Rebuilds grid from current shapes
- Use after adding/moving/editing shapes
- Preserves sources and probes

#### Simulation Info Display
- Grid dimensions (nx × ny)
- Cell size (mm)
- Current time step
- Maximum field value (for scaling)

### 6. Workflow Example

**Simulating a Waveguide:**

1. **Draw the geometry**
   - Rectangle mode: Draw outer boundary (Air)
   - Rectangle mode: Draw inner conductor (Copper)
   - Select shapes and adjust materials

2. **Add excitation**
   - "Add Point Source" at waveguide input
   - Set frequency (e.g., 10 GHz)
   - Adjust amplitude (e.g., 1.0 V/m)

3. **Add measurement**
   - "Add E-Field Probe" at output
   - "Add E-Field Probe" at midpoint

4. **Initialize and run**
   - Click "Initialize Grid"
   - Verify grid size is reasonable
   - Click "Run"
   - Watch waves propagate in real-time

5. **Analyze results**
   - Observe field patterns
   - Read probe values
   - Pause to inspect specific moments
   - Step through slowly for detail

**Simulating Reflection/Transmission:**

1. Draw three regions: Air | Dielectric | Air
2. Add plane wave source in first air region
3. Set angle = 0° (normal incidence)
4. Add probes before and after interface
5. Run simulation
6. Observe reflected and transmitted waves
7. Compare probe readings

### 7. Material Properties Impact

**Conductivity (σ):**
- High σ (metals): Strong reflection, absorption
- Low σ (dielectrics): Transmission, low loss
- Zero σ (vacuum/air): No absorption

**Permittivity (εᵣ):**
- Affects wave speed: v = c/√(εᵣ μᵣ)
- Higher εᵣ: Slower waves, shorter wavelength
- Causes refraction at interfaces

**Permeability (μᵣ):**
- Usually 1.0 for non-magnetic materials
- Affects wave impedance
- Magnetic materials: μᵣ > 1

### 8. Performance Notes

**Grid Size:**
- Larger grids = more accurate but slower
- Typical: 64×64 to 256×256 cells
- Maximum: 512×512 (may be slow)

**Cell Size:**
- Default: 1mm (good for GHz frequencies)
- Rule of thumb: λ/10 to λ/20 per cell
- Smaller cells = more accurate but more cells needed

**Time Step:**
- Automatically calculated for stability
- Courant condition: Δt ≤ Δx/(c√2)
- Cannot be manually adjusted (for safety)

**Real-time Performance:**
- Depends on grid size and CPU
- 128×128 grid: ~60 FPS on modern CPU
- 256×256 grid: ~15-30 FPS
- Use "Step" mode for detailed analysis

### 9. Keyboard Shortcuts (EM Mode)

- **Esc**: Cancel polygon drawing
- **Mouse wheel**: Zoom in/out
- **Right-drag**: Pan canvas
- **Left-click**: Select shape/source/probe
- **Left-drag handle**: Resize shape
- **Left-drag shape**: Draw new shape

### 10. Tips & Tricks

**Getting Started:**
- Start with simple geometries (1-2 shapes)
- Use presets for materials
- Initialize grid before adding sources
- Use low frequencies first (easier to visualize)

**Debugging:**
- If no waves appear: Check source is active
- If simulation crashes: Reduce grid size
- If waves look wrong: Reinitialize after shape changes
- If too slow: Reduce grid resolution

**Best Practices:**
- Add padding around shapes (grid extends beyond)
- Use contrasting materials for clear boundaries
- Place probes away from sources
- Start with "Step" mode to verify setup

**Advanced:**
- Multiple sources: Interference patterns
- Angled plane waves: Refraction studies
- High-σ shapes: Waveguide modes
- Circular shapes: Scattering analysis

### 11. Known Limitations

- 2D only (TM mode: Ez, Hx, Hy)
- No PML boundaries (waves reflect at edges)
- No dispersive materials
- No nonlinear effects
- Polygon shapes not yet editable
- No GPU acceleration (CPU only)

### 12. Future Enhancements

- [ ] 3D FDTD solver
- [ ] PML absorbing boundaries
- [ ] Frequency-domain analysis (FFT)
- [ ] S-parameter extraction
- [ ] Animation export (GIF/video)
- [ ] Field line visualization
- [ ] Poynting vector display
- [ ] GPU acceleration (CUDA/OpenCL)
- [ ] Dispersive material models
- [ ] Far-field radiation patterns

## Technical Details

### FDTD Algorithm
- Yee grid staggered in space
- Leapfrog time-stepping
- Second-order accurate in space and time
- Lossy media via conductivity term

### Equations Solved
```
∂Ez/∂t = (1/ε) * (∂Hy/∂x - ∂Hx/∂y) - (σ/ε) * Ez
∂Hx/∂t = -(1/μ) * ∂Ez/∂y
∂Hy/∂t = (1/μ) * ∂Ez/∂x
```

### Stability Condition
```
Δt ≤ Δx / (c * √2)
```
where c = 1/√(εμ) is the wave speed.

### Grid Resolution
For accurate results, use at least 10-20 cells per wavelength:
```
Δx ≤ λ / 20 = c / (20 * f)
```

## File Structure
```
ed.h              - Data structures (em_source_t, em_probe_t, fdtd_grid_t)
fdtd_solver.h     - FDTD algorithm implementation
em_editor.h       - UI and visualization
main.cpp          - Main loop with fdtd_step() call
```

## References
- Taflove & Hagness: "Computational Electrodynamics: The FDTD Method"
- Yee, K.S. (1966): "Numerical solution of initial boundary value problems"
- Sullivan, D.M.: "Electromagnetic Simulation Using the FDTD Method"
