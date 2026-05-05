#ifndef FDTD_SOLVER_H
#define FDTD_SOLVER_H

#include "ed.h"
#include <cmath>
#include <algorithm>

// Physical constants
static const float C0 = 299792458.0f;        // Speed of light (m/s)
static const float EPS0 = 8.854187817e-12f;  // Permittivity of free space (F/m)
static const float MU0 = 1.256637061e-6f;    // Permeability of free space (H/m)

// ---------------------------------------------------------------------------
// FDTD Grid Initialization
// ---------------------------------------------------------------------------

inline void fdtd_init_grid(editor_t* ed, float cell_size_meters = 0.001f) {
    fdtd_grid_t* g = &ed->fdtd;
    
    // Determine grid bounds from shapes
    float min_x = 0.f, max_x = 10.f, min_y = 0.f, max_y = 10.f;
    if (!ed->shapes.empty()) {
        min_x = max_x = ed->shapes[0].p0.x;
        min_y = max_y = ed->shapes[0].p0.y;
        for (auto& s : ed->shapes) {
            if (s.type == EM_SHAPE_RECT) {
                min_x = fminf(min_x, fminf(s.p0.x, s.p1.x));
                max_x = fmaxf(max_x, fmaxf(s.p0.x, s.p1.x));
                min_y = fminf(min_y, fminf(s.p0.y, s.p1.y));
                max_y = fmaxf(max_y, fmaxf(s.p0.y, s.p1.y));
            } else if (s.type == EM_SHAPE_CIRCLE) {
                float r = s.p1.x;
                min_x = fminf(min_x, s.p0.x - r);
                max_x = fmaxf(max_x, s.p0.x + r);
                min_y = fminf(min_y, s.p0.y - r);
                max_y = fmaxf(max_y, s.p0.y + r);
            }
        }
    }
    
    // Add padding
    float pad = 2.0f;
    min_x -= pad; max_x += pad;
    min_y -= pad; max_y += pad;

    // Grid dimensions
    g->dx = g->dy = cell_size_meters;
    g->nx = (int)((max_x - min_x) / g->dx) + 1;
    g->ny = (int)((max_y - min_y) / g->dy) + 1;

    // Clamp to reasonable size
    if (g->nx > 512) g->nx = 512;
    if (g->ny > 512) g->ny = 512;
    if (g->nx < 32) g->nx = 32;
    if (g->ny < 32) g->ny = 32;

    // Courant stability condition: dt <= dx / (c * sqrt(2))
    float max_c = C0;  // Assume vacuum for stability
    g->dt = 0.99f * g->dx / (max_c * sqrtf(2.0f));
    g->time_step = 0;

    // Allocate field arrays
    int size = g->nx * g->ny;
    g->Ez.resize(size, 0.0f);
    g->Hx.resize(size, 0.0f);
    g->Hy.resize(size, 0.0f);
    g->eps.resize(size, EPS0);
    g->mu.resize(size, MU0);
    g->sigma.resize(size, 0.0f);

    // Rasterize materials onto grid
    for (int j = 0; j < g->ny; j++) {
        for (int i = 0; i < g->nx; i++) {
            int idx = j * g->nx + i;

            // Canvas coordinates of this cell
            float cx = min_x + i * g->dx;
            float cy = min_y + j * g->dy;
            ImVec2 p(cx, cy);

            // Check which shape contains this point
            for (auto& s : ed->shapes) {
                bool inside = false;
                if (s.type == EM_SHAPE_RECT) {
                    float x0 = fminf(s.p0.x, s.p1.x);
                    float x1 = fmaxf(s.p0.x, s.p1.x);
                    float y0 = fminf(s.p0.y, s.p1.y);
                    float y1 = fmaxf(s.p0.y, s.p1.y);
                    inside = (cx >= x0 && cx <= x1 && cy >= y0 && cy <= y1);
                } else if (s.type == EM_SHAPE_CIRCLE) {
                    float dx = cx - s.p0.x;
                    float dy = cy - s.p0.y;
                    inside = (dx*dx + dy*dy) <= s.p1.x * s.p1.x;
                }

                if (inside) {
                    g->eps[idx] = s.material.permittivity * EPS0;
                    g->mu[idx] = s.material.permeability * MU0;
                    g->sigma[idx] = s.material.conductivity;
                    break;  // First shape wins
                }
            }
        }
    }

    g->initialized = true;
    g->running = false;
    g->max_field = 0.0f;
}

// ---------------------------------------------------------------------------
// FDTD Time Step (TM mode: Ez, Hx, Hy)
// ---------------------------------------------------------------------------

inline void fdtd_step(editor_t* ed) {
    fdtd_grid_t* g = &ed->fdtd;
    if (!g->initialized) return;
    
    int nx = g->nx, ny = g->ny;
    float dt = g->dt, dx = g->dx, dy = g->dy;
    
    // Update H field (magnetic)
    for (int j = 0; j < ny - 1; j++) {
        for (int i = 0; i < nx - 1; i++) {
            int idx = j * nx + i;
            int idx_x = idx + 1;
            int idx_y = idx + nx;
            
            float mu_inv = 1.0f / g->mu[idx];
            
            // Hx update: Hx += (dt/mu) * (-dEz/dy)
            g->Hx[idx] += (dt * mu_inv / dy) * (g->Ez[idx] - g->Ez[idx_y]);
            
            // Hy update: Hy += (dt/mu) * (dEz/dx)
            g->Hy[idx] += (dt * mu_inv / dx) * (g->Ez[idx_x] - g->Ez[idx]);
        }
    }
    
    // Update E field (electric) with losses
    for (int j = 1; j < ny - 1; j++) {
        for (int i = 1; i < nx - 1; i++) {
            int idx = j * nx + i;
            int idx_xm = idx - 1;
            int idx_ym = idx - nx;
            
            float eps = g->eps[idx];
            float sigma = g->sigma[idx];
            
            // Loss factor
            float ca = (2.0f * eps - sigma * dt) / (2.0f * eps + sigma * dt);
            float cb = (2.0f * dt) / (2.0f * eps + sigma * dt);
            
            // Ez update: Ez += (dt/eps) * (dHy/dx - dHx/dy) - losses
            float curl = (g->Hy[idx] - g->Hy[idx_xm]) / dx - 
                        (g->Hx[idx] - g->Hx[idx_ym]) / dy;
            
            g->Ez[idx] = ca * g->Ez[idx] + cb * curl;
        }
    }
    
    // Apply sources
    for (auto& src : ed->sources) {
        if (!src.active) continue;
        
        // Convert canvas coords to grid coords
        // (Simplified: assumes grid starts at origin)
        int gi = (int)(src.pos.x / dx);
        int gj = (int)(src.pos.y / dy);
        
        if (gi >= 0 && gi < nx && gj >= 0 && gj < ny) {
            int idx = gj * nx + gi;
            float omega = 2.0f * M_PI * src.frequency;
            float t = g->time_step * dt;
            
            if (src.type == EM_SOURCE_POINT) {
                // Gaussian pulse or sinusoidal
                g->Ez[idx] += src.amplitude * sinf(omega * t + src.phase);
            } else if (src.type == EM_SOURCE_PLANE) {
                // Plane wave along a line
                for (int k = 0; k < nx; k++) {
                    int pidx = gj * nx + k;
                    g->Ez[pidx] += src.amplitude * sinf(omega * t + src.phase);
                }
            }
        }
    }
    
    // Update probes
    for (auto& probe : ed->probes) {
        if (!probe.active) continue;
        
        int gi = (int)(probe.pos.x / dx);
        int gj = (int)(probe.pos.y / dy);
        
        if (gi >= 0 && gi < nx && gj >= 0 && gj < ny) {
            int idx = gj * nx + gi;
            if (probe.type == EM_PROBE_E_FIELD) {
                probe.value = g->Ez[idx];
            } else if (probe.type == EM_PROBE_H_FIELD) {
                probe.value = sqrtf(g->Hx[idx]*g->Hx[idx] + g->Hy[idx]*g->Hy[idx]);
            }
        }
    }
    
    // Track max field for visualization
    g->max_field = 0.0f;
    for (float val : g->Ez) {
        float abs_val = fabsf(val);
        if (abs_val > g->max_field) g->max_field = abs_val;
    }
    
    g->time_step++;
}

// ---------------------------------------------------------------------------
// Visualization
// ---------------------------------------------------------------------------

inline ImU32 field_to_color(float value, float max_val) {
    if (max_val < 1e-10f) return IM_COL32(0, 0, 0, 255);
    
    float normalized = value / max_val;
    normalized = fmaxf(-1.0f, fminf(1.0f, normalized));
    
    // Blue (negative) -> Black (zero) -> Red (positive)
    if (normalized > 0) {
        int r = (int)(255 * normalized);
        return IM_COL32(r, 0, 0, 200);
    } else {
        int b = (int)(-255 * normalized);
        return IM_COL32(0, 0, b, 200);
    }
}

inline void fdtd_draw_field(ImDrawList* dl, editor_t* ed, ImVec2 size) {
    fdtd_grid_t* g = &ed->fdtd;
    if (!g->initialized || g->Ez.empty()) return;
    
    float cell_w = size.x / g->nx;
    float cell_h = size.y / g->ny;
    
    // Draw field as colored rectangles
    for (int j = 0; j < g->ny; j++) {
        for (int i = 0; i < g->nx; i++) {
            int idx = j * g->nx + i;
            float val = g->Ez[idx];
            
            ImU32 color = field_to_color(val, g->max_field);
            
            ImVec2 p0(i * cell_w, j * cell_h);
            ImVec2 p1(p0.x + cell_w, p0.y + cell_h);
            
            dl->AddRectFilled(p0, p1, color);
        }
    }
}

#endif // FDTD_SOLVER_H
