#include "arena.h"
#include "art.h"
#include "ray.h"
#include "raylib.h"
#include "rlgl.h"
#include "ui.h"
#include "utils.h"

typedef enum {
  APP_STAGE_SCAN_GRID = 0,
  APP_STAGE_SCAN_RAYS = 1,
  APP_STAGE_SCAN_IMAGE = 2,
  APP_STAGE_RECONSTRUCTION = 3,
  APP_STAGE_ERRORS = 4,
  APP_STAGE_THANKYOU = 5,
  APP_STAGE_LOADING = 6,
} AppStage;

typedef struct {
  float offset_x;
  float offset_y;
  float zoom_level;
} StagePos;

StagePos stage_pos[6] = {
    // SCAN
    (StagePos){.offset_x = 22.67, .offset_y = 61.65f, .zoom_level = 1.46},
    (StagePos){.offset_x = 22.67, .offset_y = 61.65f, .zoom_level = 1.46},
    (StagePos){.offset_x = 22.67, .offset_y = 61.65f, .zoom_level = 1.46},
    // Reconstruction
    (StagePos){.offset_x = 22.67, .offset_y = -637.105f, .zoom_level = 1.46},
    // errors
    (StagePos){.offset_x = 22.67, .offset_y = -1341.190430f, .zoom_level = 1.46},
    // thankyou
    (StagePos){.offset_x = 191.93, .offset_y = -1589.548218f, .zoom_level = 1.23},
};

AppStage stage = APP_STAGE_SCAN_GRID;
AppStage old_stage = APP_STAGE_LOADING;

size_t GRID_CELL_SIZE = 5;
size_t NUM_SOURCES = 360;
size_t RAYS_PER_SOURCE = 30;     // Dense angular sampling
float RAYS_SPREAD_ANGLE = 30.0f; // Wide enough to cover corners

#define ITERATIONS_PER_FRAME 16

int gWidth = 640;
int gHeight = 480;

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
EM_JS(int, canvas_w, (), { return Module.canvas.clientWidth; });
EM_JS(int, canvas_h, (), { return Module.canvas.clientHeight; });
EM_JS(void, set_stage, (int app_stage), { stage = app_stage; });
EM_JS(void, hide_loader, (), { Module.setStatus(''); });

EMSCRIPTEN_KEEPALIVE
void setStage(int stageFromJs) {
  stage = stageFromJs;
}
#endif

void UpdateCanvasInfo() {
#ifdef __EMSCRIPTEN__
  /* gWidth = canvas_w(); */
  /* gHeight = canvas_h(); */
  TraceLog(LOG_INFO, "Canvas size: %d x %d", gWidth, gHeight);
#endif
}

int main(void) {
  SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
  UpdateCanvasInfo();
  InitWindow(gWidth, gHeight, "Kaczmarz Reconstruction");
  hide_loader();

  unsigned char *originalPixels = NULL;
  Image img = LoadPGM("./resources/nii_slices/slice_0128.pgm", &originalPixels);
  if (!img.data) {
    CloseWindow();
    return 1;
  }

  int img_w = img.width;
  int img_h = img.height;
  int src_idx = 0;

  UIState ui = ui_state_init();
  Arena *arena = arena_create();

  RaySet rays = rayset_generate_fan(arena, NUM_SOURCES, RAYS_PER_SOURCE, RAYS_SPREAD_ANGLE);
  ReconGrid rgrid = recon_grid_alloc(arena, img_w, img_h, GRID_CELL_SIZE);

  rayset_translate(&rays, 0, 0, img_w, img_h);
  recon_grid_build_truth(&rgrid, originalPixels, img_w, img_h);
  recon_precompute_projections(&rgrid, &rays);

  Texture2D src_tex = LoadTextureFromImage(img);

  Image recon_img = GenImageColor(img_w, img_h, BLACK);
  Color *recon_px = (Color *)recon_img.data;
  Texture2D recon_tex = LoadTextureFromImage(recon_img);

  Image error_img = GenImageColor(img_w, img_h, BLACK);
  Color *error_px = (Color *)error_img.data;
  Texture2D error_tex = LoadTextureFromImage(error_img);

  SetTargetFPS(60);

  size_t curentRayFrame = 0;

  while (!WindowShouldClose()) {
    ui_handle_input(&ui, gWidth);

    if (stage >= 2) {
      // Ensure rays are in reconstruction coordinates before iteration
      rayset_translate(&rays, 0, 0, img_w, img_h);

      for (int it = 0; it < ITERATIONS_PER_FRAME; it++) {
        recon_iterate_fan(&rgrid, &rays, src_idx);
        src_idx = (src_idx + 1) % NUM_SOURCES;
        ui.iteration++;
      }

      ui_update_recon_texture(recon_px, &rgrid, img_w, img_h);
      UpdateTexture(recon_tex, recon_px);

      ui_update_error_texture(error_px, &rgrid, img_w, img_h);
      UpdateTexture(error_tex, error_px);
    }

    BeginDrawing();
    ClearBackground(UI_BG_COLOR);

    // start zoomable canvas here
    BeginScissorMode(0, 0, gWidth, gHeight);
    rlPushMatrix();
    if (old_stage != stage) {
      ui.offset_x = stage_pos[stage].offset_x;
      ui.offset_y = stage_pos[stage].offset_y;
      ui.zoom = stage_pos[stage].zoom_level;
      old_stage = stage;
    }
    rlTranslatef(ui.offset_x, ui.offset_y, 0);
    rlScalef(ui.zoom, ui.zoom, 1.0f);

    // do zoomable graphics here

    UILayout layout = ui_compute_layout(0, 0, img_w, img_h, 0);
    if (stage >= 0) {
      const char *label = "Scanning original";
      switch (stage) {
      case APP_STAGE_SCAN_GRID:
        label = "Sample brain scan";
        break;
      case APP_STAGE_SCAN_RAYS:
        label = "Scan grid";
        break;
      case APP_STAGE_SCAN_IMAGE:
        label = "Scanning rays";
        break;
      }
      ui_draw_image_panel(src_tex, layout.x, layout.y, layout.padding, label);
    }
    if (stage >= 1) {
      ui_draw_grid_overlay(layout.innerX, layout.innerY, img_w, img_h, 256 / 5, 256 / 5, 5);
    }

    // rays scan
    if (stage == 2) {
      rayset_translate(&rays, layout.x, layout.y, layout.width, layout.height);
      ui_draw_rays(&rays, curentRayFrame);
      curentRayFrame = (curentRayFrame + 1) % NUM_SOURCES;
    }

    next_panel(&layout, 0, gHeight);
    ui_draw_image_panel(recon_tex, layout.x, layout.y, layout.padding, "Kaczmarz Reconstruction");
    DrawText(TextFormat("Iterations: %d", ui.iteration), layout.x + layout.width + layout.padding + 10, layout.innerY, 18, UI_TEXT_COLOR);
    DrawText(TextFormat("Num sources: %d", NUM_SOURCES), layout.x + layout.width + layout.padding + 10, layout.innerY + 20, 18, UI_TEXT_COLOR);
    DrawText(TextFormat("Rays per \n \tsource: %d", RAYS_PER_SOURCE), layout.x + layout.width + layout.padding + 10, layout.innerY + 40, 18, UI_TEXT_COLOR);

    next_panel(&layout, 0, gHeight);
    ui_draw_image_panel(error_tex, layout.x, layout.y, layout.padding, "Errors");
    DrawText("Red: over", layout.x + layout.width + layout.padding + 10, layout.innerY, 18, UI_TEXT_COLOR);
    DrawText("Blue: under", layout.x + layout.width + layout.padding + 10, layout.innerY + 20, 18, UI_TEXT_COLOR);

    if (stage >= 4) {
      next_panel(&layout, 0, gHeight);
      DrawText("Thank you!", layout.x, layout.y, 36, UI_TEXT_COLOR);
      DrawText("Powered by raylib", gWidth - 200, gHeight - 30, 20, UI_TEXT_COLOR);
    }

    rlPopMatrix();
    EndScissorMode();
    // end zoomable canvas

    EndDrawing();
  }

  arena_destroy(arena);
  UnloadTexture(src_tex);
  UnloadTexture(recon_tex);
  UnloadTexture(error_tex);
  UnloadImage(img);
  UnloadImage(recon_img);
  UnloadImage(error_img);
  CloseWindow();

  return 0;
}
