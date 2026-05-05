# Quick Start Guide - EM Wave Simulation

## 5-Minute Tutorial

### Step 1: Launch and Select Mode
```
./build/sim
```
- Click **"EM Waves / Medium Editor"** button

### Step 2: Draw Your First Waveguide
1. **Draw outer boundary**:
   - Mode: Rectangle (already selected)
   - Material: Air/Vacuum preset
   - Click-drag on canvas to draw large rectangle

2. **Draw conductor**:
   - Click "Copper" preset
   - Draw smaller rectangle inside
   - This creates a hollow waveguide

### Step 3: Add a Wave Source
1. In **"Sources"** panel, click **"Add Point Source"**
2. Source appears on canvas (red pulsing circle)
3. Click and drag it to the left side of your waveguide
4. Select it and adjust:
   - Frequency: 1e9 (1 GHz)
   - Amplitude: 1.0

### Step 4: Add Field Probes
1. In **"Probes"** panel, click **"Add E-Field Probe"** twice
2. Drag first probe near the source
3. Drag second probe to the right side
4. These will measure field strength

### Step 5: Run Simulation
1. Click **"Initialize Grid"** in Simulation panel
   - Wait 1-2 seconds for grid generation
   - Status shows grid size (e.g., "128x128 cells")

2. Click **"Run"**
   - Watch waves propagate in real-time!
   - Red/blue colors show field intensity
   - Probes display live measurements

3. Controls:
   - **Pause**: Freeze the simulation
   - **Step**: Advance one frame
   - **Reset**: Start over from t=0

### Step 6: Edit Your Geometry
1. Click on a shape to select it
2. White circles appear (resize handles)
3. Drag any handle to resize
4. Click **"Reinitialize"** to update simulation

### Step 7: Experiment!
- Try different materials (PTFE, FR4, Water)
- Add more sources (interference patterns!)
- Change source frequency
- Add plane wave source
- Measure fields at different locations

---

## Common Scenarios

### Scenario 1: Reflection at Interface
**Goal**: See waves reflect and transmit at material boundary

```
1. Draw three rectangles side-by-side:
   - Left: Air (εᵣ=1.0)
   - Middle: FR4 (εᵣ=4.5)
   - Right: Air (εᵣ=1.0)

2. Add plane wave source in left region:
   - Angle: 0° (normal incidence)
   - Frequency: 5e9 (5 GHz)

3. Add probes:
   - One before interface
   - One after interface

4. Initialize and Run
5. Observe: Partial reflection + transmission
```

### Scenario 2: Waveguide Modes
**Goal**: Excite and visualize waveguide modes

```
1. Draw rectangular waveguide:
   - Outer: Air (large rectangle)
   - Inner: Copper conductor (smaller rectangle)
   - Make it long and narrow

2. Add point source at one end (centered)
3. Set frequency to match waveguide cutoff
4. Add probes along the length
5. Run and observe standing wave patterns
```

### Scenario 3: Scattering from Cylinder
**Goal**: See waves scatter around obstacle

```
1. Draw background: Large air rectangle
2. Draw obstacle: Copper circle in center
3. Add plane wave from left
4. Add probes around the circle
5. Run and watch diffraction pattern
```

---

## Keyboard & Mouse Reference

### Mouse
- **Left-click**: Select object
- **Left-drag**: Draw shape / Move object / Resize via handle
- **Right-drag**: Pan canvas
- **Scroll**: Zoom in/out
- **Double-click**: Close polygon (polygon mode only)

### Keyboard
- **Esc**: Cancel polygon drawing

### UI Panels
- **EM Material Palette**: Left sidebar
  - Draw mode selection
  - Material properties
  - Presets
  - Shape editor
  - Simulation controls
  - Sources list
  - Probes list

- **EM Canvas**: Main area
  - Drawing surface
  - Field visualization
  - Object rendering

---

## Troubleshooting

### "No waves appear"
- ✓ Check source is **Active** (checkbox)
- ✓ Verify source is inside simulation bounds
- ✓ Try higher amplitude (e.g., 10.0)
- ✓ Click "Reset" and "Run" again

### "Simulation is too slow"
- ✓ Grid might be too large (check status)
- ✓ Click "Reinitialize" with fewer shapes
- ✓ Use "Step" mode instead of "Run"
- ✓ Reduce canvas zoom (smaller visible area)

### "Waves look wrong after editing"
- ✓ Click **"Reinitialize"** after shape changes
- ✓ Grid doesn't auto-update when shapes move
- ✓ Sources/probes may need repositioning

### "Can't resize shape"
- ✓ Click shape first to select it
- ✓ Handles appear as white circles
- ✓ Click directly on handle, then drag
- ✓ Polygon editing not yet implemented

### "Simulation crashes/freezes"
- ✓ Grid too large (>512×512)
- ✓ Reduce number of shapes
- ✓ Restart application
- ✓ Use simpler geometry

---

## Tips for Best Results

### Geometry
- ✅ Start simple (1-3 shapes)
- ✅ Leave padding around edges
- ✅ Use contrasting materials
- ✅ Avoid very thin shapes (< 2 cells wide)

### Sources
- ✅ Place away from boundaries
- ✅ Start with low frequency (easier to see)
- ✅ Use amplitude ~1.0 initially
- ✅ One source first, add more later

### Probes
- ✅ Place away from sources
- ✅ Use multiple probes to compare
- ✅ Watch values stabilize over time
- ✅ Scientific notation is normal (e.g., 1.23e-5)

### Simulation
- ✅ Initialize before adding sources (optional)
- ✅ Use "Step" to verify setup
- ✅ "Run" for continuous animation
- ✅ "Pause" to inspect details
- ✅ Reinitialize after major changes

### Performance
- ✅ Smaller grid = faster (but less accurate)
- ✅ Typical: 128×128 is good balance
- ✅ 256×256 for detailed studies
- ✅ Avoid 512×512 unless necessary

---

## Next Steps

After mastering the basics:

1. **Read EM_FEATURES.md** for complete feature list
2. **Try advanced scenarios** (multiple sources, angled waves)
3. **Experiment with materials** (conductors vs dielectrics)
4. **Measure and analyze** (probe data, field patterns)
5. **Explore circuit mode** (switch via View menu)

---

## Getting Help

- **Documentation**: See EM_FEATURES.md
- **Architecture**: See ARCHITECTURE.md
- **Changes**: See CHANGELOG.md
- **Code**: Headers are well-commented

## Have Fun!

Electromagnetic wave simulation is fascinating. Experiment, break things, learn, and enjoy watching Maxwell's equations come to life! 🌊⚡
