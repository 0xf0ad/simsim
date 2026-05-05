#ifndef EM_EDITOR_H
#define EM_EDITOR_H

#include "ed.h"
#include "fdtd_solver.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include <GLFW/glfw3.h>
#include <vector>
#include <math.h>
#include <string.h>

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

inline ImVec2 em_canvas_to_screen(const editor_t* c, ImVec2 p) {
    return ImVec2(c->grid.offset[0] + p.x * c->grid.scale,
                  c->grid.offset[1] + p.y * c->grid.scale);
}

inline ImVec2 em_screen_to_canvas(const editor_t* c, ImVec2 s, ImVec2 origin) {
    return ImVec2((s.x - origin.x - c->grid.offset[0]) / c->grid.scale,
                  (s.y - origin.y - c->grid.offset[1]) / c->grid.scale);
}

// ---------------------------------------------------------------------------
// Preset materials
// ---------------------------------------------------------------------------

static const em_material_t em_presets[] = {
    { 5.8e7f,  1.0f, 1.0f, "Copper",    ImVec4(1.00f, 0.75f, 0.20f, 0.55f) },
    { 3.5e7f,  1.0f, 1.0f, "Aluminum",  ImVec4(0.75f, 0.75f, 0.80f, 0.55f) },
    { 1.0e7f,  1.0f, 1.0f, "Iron",      ImVec4(0.55f, 0.55f, 0.60f, 0.55f) },
    { 0.0f,    2.2f, 1.0f, "PTFE",      ImVec4(0.90f, 0.90f, 0.95f, 0.45f) },
    { 0.0f,    4.5f, 1.0f, "FR4",       ImVec4(0.20f, 0.65f, 0.30f, 0.45f) },
    { 0.0f,    1.0f, 1.0f, "Air/Vacuum",ImVec4(0.50f, 0.80f, 1.00f, 0.15f) },
    { 1.0f,    80.f, 1.0f, "Water",     ImVec4(0.20f, 0.50f, 0.90f, 0.40f) },
};
static const int em_num_presets = (int)(sizeof(em_presets)/sizeof(em_presets[0]));

// ---------------------------------------------------------------------------
// Hit-testing
// ---------------------------------------------------------------------------

inline bool em_point_in_rect(ImVec2 p, ImVec2 a, ImVec2 b) {
    float x0 = a.x < b.x ? a.x : b.x;
    float x1 = a.x > b.x ? a.x : b.x;
    float y0 = a.y < b.y ? a.y : b.y;
    float y1 = a.y > b.y ? a.y : b.y;
    return p.x >= x0 && p.x <= x1 && p.y >= y0 && p.y <= y1;
}

inline bool em_point_in_circle(ImVec2 p, ImVec2 centre, float r) {
    float dx = p.x - centre.x, dy = p.y - centre.y;
    return (dx*dx + dy*dy) <= r*r;
}

// Simple point-in-polygon (ray casting)
inline bool em_point_in_polygon(ImVec2 p, const std::vector<ImVec2>& pts) {
    bool inside = false;
    size_t n = pts.size();
    for (size_t i = 0, j = n - 1; i < n; j = i++) {
        float xi = pts[i].x, yi = pts[i].y;
        float xj = pts[j].x, yj = pts[j].y;
        if (((yi > p.y) != (yj > p.y)) &&
            (p.x < (xj - xi) * (p.y - yi) / (yj - yi) + xi))
            inside = !inside;
    }
    return inside;
}

inline bool em_hit_shape(const em_shape_t* s, ImVec2 p) {
    switch (s->type) {
        case EM_SHAPE_RECT:
            return em_point_in_rect(p, s->p0, s->p1);
        case EM_SHAPE_CIRCLE:
            return em_point_in_circle(p, s->p0, s->p1.x);
        case EM_SHAPE_POLYGON:
            return em_point_in_polygon(p, s->poly_pts);
    }
    return false;
}

// ---------------------------------------------------------------------------
// Drawing helpers
// ---------------------------------------------------------------------------

inline void em_draw_source(ImDrawList* dl, const editor_t* c, const em_source_t* src, bool selected) {
    ImVec2 screen_pos = em_canvas_to_screen(c, src->pos);
    ImU32 color = selected ? IM_COL32(255, 255, 0, 255) : IM_COL32(255, 100, 100, 255);
    
    if (src->type == EM_SOURCE_POINT) {
        // Draw as pulsing circle
        float r = 8.0f + 3.0f * sinf(ImGui::GetTime() * 4.0f);
        dl->AddCircleFilled(screen_pos, r, color);
        dl->AddCircle(screen_pos, r + 2, IM_COL32(255, 255, 255, 200), 16, 2.0f);
        // Label
        dl->AddText(ImVec2(screen_pos.x + 12, screen_pos.y - 7), 
                   IM_COL32(255, 255, 255, 255), src->name);
    } else if (src->type == EM_SOURCE_PLANE) {
        // Draw as arrow indicating direction
        float len = 30.0f;
        float dx = cosf(src->angle) * len;
        float dy = sinf(src->angle) * len;
        ImVec2 end(screen_pos.x + dx, screen_pos.y + dy);
        dl->AddLine(screen_pos, end, color, 3.0f);
        // Arrowhead
        float angle = src->angle + M_PI;
        ImVec2 a1(end.x + 10*cosf(angle + 0.5f), end.y + 10*sinf(angle + 0.5f));
        ImVec2 a2(end.x + 10*cosf(angle - 0.5f), end.y + 10*sinf(angle - 0.5f));
        dl->AddTriangleFilled(end, a1, a2, color);
        dl->AddText(ImVec2(screen_pos.x + 12, screen_pos.y - 7), 
                   IM_COL32(255, 255, 255, 255), src->name);
    }
}

inline void em_draw_probe(ImDrawList* dl, const editor_t* c, const em_probe_t* probe, bool selected) {
    ImVec2 screen_pos = em_canvas_to_screen(c, probe->pos);
    ImU32 color = selected ? IM_COL32(255, 255, 0, 255) : IM_COL32(100, 255, 100, 255);
    
    // Draw as square
    float sz = 6.0f;
    dl->AddRectFilled(ImVec2(screen_pos.x - sz, screen_pos.y - sz),
                     ImVec2(screen_pos.x + sz, screen_pos.y + sz), color);
    dl->AddRect(ImVec2(screen_pos.x - sz - 1, screen_pos.y - sz - 1),
               ImVec2(screen_pos.x + sz + 1, screen_pos.y + sz + 1),
               IM_COL32(255, 255, 255, 200), 0, 0, 2.0f);
    
    // Label with value
    char label[64];
    snprintf(label, 64, "%s: %.2e", probe->name, probe->value);
    dl->AddText(ImVec2(screen_pos.x + 10, screen_pos.y - 7), 
               IM_COL32(255, 255, 255, 255), label);
}

inline void em_draw_shape(ImDrawList* dl, const editor_t* c,
                          const em_shape_t* s, bool highlight) {
    ImU32 fill   = ImGui::ColorConvertFloat4ToU32(s->material.color);
    ImU32 border = highlight
        ? IM_COL32(255, 220, 60, 255)
        : IM_COL32(200, 200, 200, 200);
    float thick = highlight ? 2.5f : 1.5f;

    switch (s->type) {
        case EM_SHAPE_RECT: {
            ImVec2 a = em_canvas_to_screen(c, s->p0);
            ImVec2 b = em_canvas_to_screen(c, s->p1);
            dl->AddRectFilled(a, b, fill);
            dl->AddRect(a, b, border, 0.0f, 0, thick);
            dl->AddText(a, IM_COL32(255,255,255,200), s->material.name);
            break;
        }
        case EM_SHAPE_CIRCLE: {
            ImVec2 ctr = em_canvas_to_screen(c, s->p0);
            float  r   = s->p1.x * c->grid.scale;
            dl->AddCircleFilled(ctr, r, fill);
            dl->AddCircle(ctr, r, border, 64, thick);
            dl->AddText(ImVec2(ctr.x - 20, ctr.y - 7),
                        IM_COL32(255,255,255,200), s->material.name);
            break;
        }
        case EM_SHAPE_POLYGON: {
            if (s->poly_pts.size() < 2) break;
            // convert to screen
            std::vector<ImVec2> scr;
            scr.reserve(s->poly_pts.size());
            for (auto& v : s->poly_pts)
                scr.push_back(em_canvas_to_screen(c, v));
            dl->AddConvexPolyFilled(scr.data(), (int)scr.size(), fill);
            dl->AddPolyline(scr.data(), (int)scr.size(), border, ImDrawFlags_Closed, thick);
            if (!scr.empty())
                dl->AddText(scr[0], IM_COL32(255,255,255,200), s->material.name);
            break;
        }
    }
}

// Draw resize handles for selected shape
inline void em_draw_edit_handles(ImDrawList* dl, const editor_t* c, const em_shape_t* s){
    if (s->type == EM_SHAPE_RECT) {
        ImVec2 a = em_canvas_to_screen(c, s->p0);
        ImVec2 b = em_canvas_to_screen(c, s->p1);

        // 8 handles: 4 corners + 4 edges
        ImVec2 handles[8] = {
            ImVec2(a.x, a.y),                    // 0: top-left
            ImVec2((a.x+b.x)*0.5f, a.y),        // 1: top-mid
            ImVec2(b.x, a.y),                    // 2: top-right
            ImVec2(b.x, (a.y+b.y)*0.5f),        // 3: right-mid
            ImVec2(b.x, b.y),                    // 4: bottom-right
            ImVec2((a.x+b.x)*0.5f, b.y),        // 5: bottom-mid
            ImVec2(a.x, b.y),                    // 6: bottom-left
            ImVec2(a.x, (a.y+b.y)*0.5f),        // 7: left-mid
        };
        
        for (int i = 0; i < 8; i++) {
            dl->AddCircleFilled(handles[i], 5.0f, IM_COL32(255, 255, 255, 255));
            dl->AddCircle(handles[i], 5.0f, IM_COL32(0, 0, 0, 255), 12, 1.5f);
        }
    } else if (s->type == EM_SHAPE_CIRCLE) {
        ImVec2 ctr = em_canvas_to_screen(c, s->p0);
        float r = s->p1.x * c->grid.scale;
        
        // 4 handles on cardinal directions
        ImVec2 handles[4] = {
            ImVec2(ctr.x + r, ctr.y),
            ImVec2(ctr.x, ctr.y + r),
            ImVec2(ctr.x - r, ctr.y),
            ImVec2(ctr.x, ctr.y - r),
        };
        
        for (int i = 0; i < 4; i++) {
            dl->AddCircleFilled(handles[i], 5.0f, IM_COL32(255, 255, 255, 255));
            dl->AddCircle(handles[i], 5.0f, IM_COL32(0, 0, 0, 255), 12, 1.5f);
        }
    }
}

// Check if mouse is over an edit handle
inline int em_hit_edit_handle(const editor_t* c, const em_shape_t* s, ImVec2 mouse_canvas, float threshold = 0.15f) {
    if (s->type == EM_SHAPE_RECT) {
        ImVec2 handles[8] = {
            s->p0,
            ImVec2((s->p0.x + s->p1.x) * 0.5f, s->p0.y),
            ImVec2(s->p1.x, s->p0.y),
            ImVec2(s->p1.x, (s->p0.y + s->p1.y) * 0.5f),
            s->p1,
            ImVec2((s->p0.x + s->p1.x) * 0.5f, s->p1.y),
            ImVec2(s->p0.x, s->p1.y),
            ImVec2(s->p0.x, (s->p0.y + s->p1.y) * 0.5f),
        };
        
        for (int i = 0; i < 8; i++) {
            float dx = mouse_canvas.x - handles[i].x;
            float dy = mouse_canvas.y - handles[i].y;
            if (dx*dx + dy*dy < threshold*threshold) return i;
        }
    } else if (s->type == EM_SHAPE_CIRCLE) {
        float r = s->p1.x;
        ImVec2 handles[4] = {
            ImVec2(s->p0.x + r, s->p0.y),
            ImVec2(s->p0.x, s->p0.y + r),
            ImVec2(s->p0.x - r, s->p0.y),
            ImVec2(s->p0.x, s->p0.y - r),
        };
        
        for (int i = 0; i < 4; i++) {
            float dx = mouse_canvas.x - handles[i].x;
            float dy = mouse_canvas.y - handles[i].y;
            if (dx*dx + dy*dy < threshold*threshold) return i;
        }
    }
    return -1;
}

#endif // EM_EDITOR_H
