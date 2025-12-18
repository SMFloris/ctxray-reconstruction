#pragma once

#include "art.h"
#include "ray.h"
#include "raylib.h"
#include <stdio.h>

#ifdef __EMSCRIPTEN__
#warning "EMSCRIPTEN ENABLED"
#include <emscripten.h>

EM_JS(void, showModalInfoFromC, (int x), {
  console.log("JS got:", x);
  window.modalShow(x); // any JS you defined in your HTML
});

#endif

// UI Colors
#define UI_BG_COLOR ((Color){30, 30, 35, 255})
#define UI_PANEL_COLOR ((Color){45, 45, 50, 255})
#define UI_BORDER_COLOR ((Color){70, 70, 80, 255})
#define UI_TEXT_COLOR ((Color){220, 220, 220, 255})
#define UI_TEXT_DIM_COLOR ((Color){140, 140, 150, 255})
#define UI_ACCENT_COLOR ((Color){100, 180, 255, 255})
#define UI_RAY_COLOR ((Color){255, 100, 100, 200})
#define UI_GRID_COLOR ((Color){100, 180, 255, 60})

// Layout constants
#define UI_TOP_BAR_HEIGHT 50
#define UI_IMAGE_PADDING 50
#define UI_IMAGE_GAP 30

// UI State
typedef struct {
  bool show_grid;
  bool show_rays;
  bool paused;
  bool panning;
  Vector2 pan_start;
  bool mode_changed;
  float zoom;
  float offset_x, offset_y;
  int iteration;
  bool show_info_modal;
  int img_w, img_h;
} UIState;

// Initialize UI state
static inline UIState ui_state_init(void) {
  return (UIState){
      .show_grid = false,
      .show_rays = true,
      .paused = false,
      .mode_changed = false,
      .iteration = 0,
      .zoom = 1.0f,
      .offset_x = 0.0f,
      .offset_y = 0.0f,
      .show_info_modal = false,
      .img_w = 0,
      .img_h = 0};
}

// Handle UI input, returns true if ray mode changed
static inline void ui_handle_input(UIState *ui, int screen_w) {
  ui->mode_changed = false;

  if (IsKeyPressed(KEY_G))
    ui->show_grid = !ui->show_grid;
  if (IsKeyPressed(KEY_R))
    ui->show_rays = !ui->show_rays;
  if (IsKeyPressed(KEY_SPACE))
    ui->paused = !ui->paused;

  // --- Zoom (cursor-centered) ---
  float wheel = GetMouseWheelMove();
  if (wheel != 0.0f) {
    Vector2 mouse = GetMousePosition();

    float old_zoom = ui->zoom;
    ui->zoom *= (1.0f + wheel * 0.1f);
    if (ui->zoom < 0.1f)
      ui->zoom = 0.1f;
    if (ui->zoom > 5.0f)
      ui->zoom = 5.0f;

    float z = ui->zoom / old_zoom;

    // Maintain cursor focus point
    ui->offset_x = mouse.x - (mouse.x - ui->offset_x) * z;
    ui->offset_y = mouse.y - (mouse.y - ui->offset_y) * z;
  }

  // --- Panning ---
  if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)) {
    ui->panning = true;
    ui->pan_start = GetMousePosition();
  }

  if (ui->panning) {
    Vector2 mouse = GetMousePosition();
    Vector2 delta = {mouse.x - ui->pan_start.x, mouse.y - ui->pan_start.y};

    ui->offset_x += delta.x;
    ui->offset_y += delta.y;

    ui->pan_start = mouse;

    if (IsMouseButtonReleased(MOUSE_BUTTON_MIDDLE)) {
      ui->panning = false;
      TraceLog(LOG_INFO, "Drag end: offset_x=%f, offset_y=%f, zoom=%f", ui->offset_x, ui->offset_y, ui->zoom);
    }
  }

  // Info button click
  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    Vector2 mouse = GetMousePosition();
    int bx = screen_w - 150;
    int by = 10;
    int bw = 140;
    int bh = 30;
    if (mouse.x >= bx && mouse.x <= bx + bw && mouse.y >= by && mouse.y <= by + bh) {
      ui->show_info_modal = !ui->show_info_modal;
      if (ui->show_info_modal) {
#ifdef __EMSCRIPTEN__
        showModalInfoFromC(10);
#endif
      }
    }
  }
}

// Draw grid overlay on an image
static inline void ui_draw_grid_overlay(int ox, int oy, int w, int h, int nx, int ny, int cell_size) {
  for (int i = 0; i <= nx; i++) {
    int x = ox + i * cell_size;
    DrawLine(x, oy, x, oy + h, UI_GRID_COLOR);
  }
  for (int i = 0; i <= ny; i++) {
    int y = oy + i * cell_size;
    DrawLine(ox, y, ox + w, y, UI_GRID_COLOR);
  }
}

// Draw image panel with border and label
static inline void ui_draw_image_panel(Texture2D tex, int x, int y, int padding, const char *label) {
  Rectangle border = {(float)(x - padding), (float)(y - padding),
                      (float)(tex.width + padding * 2),
                      (float)(tex.height + padding * 2)};

  DrawRectangleRec(border, BLACK);
  DrawRectangleLinesEx(border, 1, UI_BORDER_COLOR);
  DrawTexture(tex, x, y, WHITE);
  DrawText(label, x, y - padding - 22, 18, UI_TEXT_COLOR);
}

// Draw top bar
static inline void ui_draw_top_bar(int screen_w, const UIState *ui, int img_w, int img_h, int nx, int ny) {
  DrawRectangle(0, 0, screen_w, UI_TOP_BAR_HEIGHT, UI_PANEL_COLOR);
  DrawLine(0, UI_TOP_BAR_HEIGHT, screen_w, UI_TOP_BAR_HEIGHT, UI_BORDER_COLOR);

  int tx = 20;
  int ty = 16;

  DrawText("ART Reconstruction", tx, ty, 18, UI_ACCENT_COLOR);
  tx += 220;

  DrawText("[G] Grid", tx, ty, 14, ui->show_grid ? UI_ACCENT_COLOR : UI_TEXT_DIM_COLOR);
  tx += 90;

  DrawText("[Space] Pause", tx, ty, 14, ui->paused ? UI_ACCENT_COLOR : UI_TEXT_DIM_COLOR);
  tx += 120;

  char buf[128];
  snprintf(buf, sizeof(buf), "Iter: %d  |  Image: %dx%d  |  Grid: %dx%d", ui->iteration, img_w, img_h, nx, ny);
  DrawText(buf, tx, ty, 14, UI_TEXT_DIM_COLOR);

  DrawFPS(screen_w - 80, ty);

  // Info button
  int bx = screen_w - 150 - 80;
  DrawRectangle(bx, 10, 140, 30, UI_PANEL_COLOR);
  DrawRectangleLines(bx, 10, 140, 30, UI_BORDER_COLOR);
  DrawText("Show/Hide Info", bx + 10, 16, 14, UI_TEXT_COLOR);
}

// Draw rays visualization
static inline void ui_draw_rays(const RaySet *rs, size_t iteration) {
  if (rs->type != RAY_MODE_FAN) {
    TraceLog(LOG_INFO, "NOT FAN");
    return;
  }
  size_t startIndex = iteration * rs->metadata.fan.num_rays_per_source;
  size_t endIndex = startIndex + rs->metadata.fan.num_rays_per_source;

  CTRay ray;
  for (size_t i = startIndex; i < endIndex; i++) {
    ray = rs->rays[i];

    Vector2 p1 = {ray.ox, ray.oy};
    Vector2 p2 = {ray.ox + ray.dx * ray.length, ray.oy + ray.dy * ray.length};
    // TraceLog(LOG_INFO, "RAY %d (%f,%f) -> (%f %f)", i, ray.ox, ray.oy, p2.x, p2.y);
    DrawLineEx(p1, p2, 1.5f, UI_RAY_COLOR);
  }

  DrawCircle(ray.ox, ray.oy, 5, UI_RAY_COLOR);
  // debugging circle - fuck trig
  DrawCircleLines(rs->metadata.fan.cx, rs->metadata.fan.cy, rs->metadata.fan.radius, UI_RAY_COLOR);
}

// Update reconstruction image from grid values
static inline void ui_update_recon_texture(Color *pixels, const ReconGrid *g, int img_w, int img_h) {
  for (int iy = 0; iy < g->ny; iy++) {
    for (int ix = 0; ix < g->nx; ix++) {
      float val = g->values[iy * g->nx + ix];

      // Scale from [0,1] back to [0,255] for display
      unsigned char v = (unsigned char)(val * 255.0f);

      for (int yy = 0; yy < g->cell_size; yy++) {
        for (int xx = 0; xx < g->cell_size; xx++) {
          int px = ix * g->cell_size + xx;
          int py = iy * g->cell_size + yy;
          if (px < img_w && py < img_h)
            pixels[py * img_w + px] = (Color){v, v, v, 255};
        }
      }
    }
  }
}

// Update error image (blue = under, red = over)
static inline void ui_update_error_texture(Color *pixels, const ReconGrid *g, int img_w, int img_h) {
  for (int iy = 0; iy < g->ny; iy++) {
    for (int ix = 0; ix < g->nx; ix++) {
      float err = g->values[iy * g->nx + ix] - g->ground_truth[iy * g->nx + ix];
      float scaled = err * 400.0f;

      unsigned char r, gc, b;
      if (scaled > 0) {
        r = (unsigned char)fminf(scaled, 255.0f);
        gc = 0;
        b = 0;
      } else {
        r = 0;
        gc = 0;
        b = (unsigned char)fminf(-scaled, 255.0f);
      }

      for (int yy = 0; yy < g->cell_size; yy++) {
        for (int xx = 0; xx < g->cell_size; xx++) {
          int px = ix * g->cell_size + xx;
          int py = iy * g->cell_size + yy;
          if (px < img_w && py < img_h)
            pixels[py * img_w + px] = (Color){r, gc, b, 255};
        }
      }
    }
  }
}

// Compute layout positions for three images
typedef struct {
  int x, y;
  int height, width;
  int innerX, innerY;
  int padding;
} UILayout;

static inline UILayout ui_compute_layout(int x, int y, int w, int h, int padding) {
  UILayout l;
  l.padding = padding;
  l.height = h;
  l.width = w;
  l.x = x;
  l.y = y;
  l.innerX = x + padding;
  l.innerY = y + padding;

  return l;
}

static inline void next_panel(UILayout *l, int marginLeft, int marginTop) {
  l->x = l->x + marginLeft;
  l->y = l->y + marginTop;
  l->innerX = l->x + l->padding;
  l->innerY = l->y + l->padding;
}
