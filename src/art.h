#pragma once

#include "arena.h"
#include "ray.h"
#include "utils.h"
#include <math.h>

// Reconstruction grid
typedef struct {
  float *values;       // Current reconstruction values
  float *ground_truth; // Ground truth (from source image)
  float *row_buffer;   // Scratch buffer for system matrix row
  int nx, ny;          // Grid dimensions
  int cell_size;       // Pixels per cell
  int n;               // Total cells (nx * ny)
} ReconGrid;

// Allocate reconstruction grid
static inline ReconGrid recon_grid_alloc(Arena *arena, int img_w, int img_h, int cell_size) {
  ReconGrid g;
  g.cell_size = cell_size;
  g.nx = (img_w + cell_size - 1) / cell_size;
  g.ny = (img_h + cell_size - 1) / cell_size;
  g.n = g.nx * g.ny;
  g.values = (float *)arena_alloc_zero(arena, g.n * sizeof(float));
  g.ground_truth = (float *)arena_alloc(arena, g.n * sizeof(float));
  g.row_buffer = (float *)arena_alloc(arena, g.n * sizeof(float));
  return g;
}

// Build ground truth grid from source image (downsample)
// Values are normalized to [0, 1] range for numerical stability
static inline void recon_grid_build_truth(ReconGrid *g, const unsigned char *pixels, int img_w, int img_h) {
  for (int iy = 0; iy < g->ny; iy++) {
    for (int ix = 0; ix < g->nx; ix++) {
      float sum = 0.0f;
      int count = 0;

      for (int yy = 0; yy < g->cell_size; yy++) {
        for (int xx = 0; xx < g->cell_size; xx++) {
          int px = ix * g->cell_size + xx;
          int py = iy * g->cell_size + yy;
          if (px < img_w && py < img_h) {
            sum += pixels[py * img_w + px];
            count++;
          }
        }
      }

      g->ground_truth[iy * g->nx + ix] = sum / count / 255.0f;
    }
  }
}

// Build system matrix row for a single ray
static inline void recon_build_row(ReconGrid *g, const CTRay *ray) {
  for (int i = 0; i < g->n; i++)
    g->row_buffer[i] = 0.0f;

  for (int iy = 0; iy < g->ny; iy++) {
    for (int ix = 0; ix < g->nx; ix++) {
      Rect cell = {(float)(ix * g->cell_size), (float)(iy * g->cell_size),
                   (float)((ix + 1) * g->cell_size), (float)((iy + 1) * g->cell_size)};

      LiangBarskyResult hit =
          liang_barsky_ray(&cell, ray->ox, ray->oy, ray->dx, ray->dy);
      if (hit.intersects) {
        // Normalize by cell size so weights are ~1 per cell instead of ~4
        g->row_buffer[iy * g->nx + ix] = hit.length / (float)g->cell_size;
      }
    }
  }
}

// Compute projection value for a ray (dot product with ground truth)
static inline float recon_compute_projection(ReconGrid *g, const CTRay *ray) {
  recon_build_row(g, ray);

  float b = 0.0f;
  for (int i = 0; i < g->n; i++)
    b += g->ground_truth[i] * g->row_buffer[i];
  return b;
}

// Classic Kaczmarz iteration step
static inline void recon_kaczmarz_step(ReconGrid *g, float projection) {
  float ax = 0.0f, norm_a = 0.0f;

  for (int i = 0; i < g->n; i++) {
    ax += g->row_buffer[i] * g->values[i];
    norm_a += g->row_buffer[i] * g->row_buffer[i];
  }

  if (norm_a < 1e-12f)
    return;

  float alpha = (projection - ax) / norm_a;

  for (int i = 0; i < g->n; i++) {
    g->values[i] += alpha * g->row_buffer[i];
  }
}

// Process a single ray: build row, then apply Kaczmarz
static inline void recon_process_ray(ReconGrid *g, const CTRay *ray, float projection) {
  recon_build_row(g, ray);
  recon_kaczmarz_step(g, projection);
}

// Precompute all projections for a ray set
static inline void recon_precompute_projections(ReconGrid *g, RaySet *rs) {
  for (size_t i = 0; i < rs->count; i++) {
    rs->projections[i] = recon_compute_projection(g, &rs->rays[i]);
  }
}

// Run one iteration over all rays from a fan source
static inline void recon_iterate_fan(ReconGrid *g, const RaySet *rs, size_t iteration) {
  if (rs->type != RAY_MODE_FAN) {
    return;
  }
  size_t startIndex = iteration * rs->metadata.fan.num_rays_per_source;
  size_t endIndex = startIndex + rs->metadata.fan.num_rays_per_source;
  for (size_t i = startIndex; i < endIndex; i++) {
    recon_process_ray(g, &rs->rays[i], rs->projections[i]);
  }
}
