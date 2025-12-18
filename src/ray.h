#pragma once

#include "arena.h"
#include "utils.h"
#include <math.h>

// Ray mode types
typedef enum {
  RAY_MODE_FAN = 0,
  RAY_MODE_PARALLEL
} RaySetType;

typedef struct {
  float cx, cy;
  float radius;
  size_t num_sources;
  size_t num_rays_per_source;
  float angle_spread_rad;
} RaySetFanMetadata;

bool fan_same(RaySetFanMetadata *rf, float cx, float cy, float radius) {
  return rf->cx == cx && rf->cy == cy && rf->radius == radius;
}

// Single ray definition (named CTRay to avoid conflict with raylib's Ray)
typedef struct {
  float ox, oy; // Origin
  float dx, dy; // Direction, normalized
  float length; // length from origin to the detector
} CTRay;

// Collection of rays with associated data
typedef struct {
  CTRay *rays;
  float *projections;
  size_t count;
  size_t max_count;
  RaySetType type;
  union {
    RaySetFanMetadata fan;
  } metadata;
} RaySet;

void rayset_fan_translate(RaySet *rs, float new_cx, float new_cy, float new_radius) {
  RaySetFanMetadata rf = rs->metadata.fan;

  float old_cx = rf.cx;
  float old_cy = rf.cy;
  float old_r = rf.radius;

  for (size_t i = 0; i < rs->count; i++) {
    float ox = rs->rays[i].ox;
    float oy = rs->rays[i].oy;

    // recover normalized coords relative to old center
    float nx = (ox - old_cx) / old_r;
    float ny = (oy - old_cy) / old_r;

    // rebuild world coords with new center and radius
    rs->rays[i].ox = new_cx + nx * new_radius;
    rs->rays[i].oy = new_cy + ny * new_radius;

    // direction vectors are unit vectors, don't need scaling
    rs->rays[i].length = new_radius * 2.0f;
  }

  rf.cx = new_cx;
  rf.cy = new_cy;
  rf.radius = new_radius;
  rs->metadata.fan = rf;
}

// translate rayset to a new bounding box
// do nothing if bounding box is same
bool rayset_translate(RaySet *rs, int x, int y, int w, int h) {
  float cx = x + w / 2.0f;
  float cy = y + h / 2.0f;
  float radius = 0.5f * fmaxf(w, h);

  // TODO: rayset type
  if (!fan_same(&rs->metadata.fan, cx, cy, radius)) {
    rayset_fan_translate(rs, cx, cy, radius);
  }

  return false;
}

// Allocate a ray set
RaySet rayset_alloc(Arena *arena, size_t count_rays) {
  RaySet rs;
  rs.rays = (CTRay *)arena_alloc(arena, count_rays * sizeof(CTRay));
  rs.projections = (float *)arena_alloc(arena, count_rays * sizeof(float));
  rs.count = 0;
  rs.max_count = count_rays;
  return rs;
}

void rayset_append(RaySet *rs, CTRay ray) {
  rs->rays[rs->count] = ray;
  rs->count += 1;
}

// Generate fan beam ray set
RaySet rayset_generate_fan(Arena *arena, size_t num_sources, size_t num_rays_per_source, float angle_spread_deg) {
  int halfNumRays = num_rays_per_source / 2;
  size_t actual_rays_per_source = 2 * halfNumRays + 1;

  RaySet rs = rayset_alloc(arena, num_sources * actual_rays_per_source);

  float radius = 1.0f;
  float angle_spread_rad = angle_spread_deg * PI / 180.0f;
  float angle_step_deg = 360.0f / (float)num_sources;
  float angle_step = angle_step_deg * PI / 180.0f;

  float spread_angle_step = angle_spread_rad / (float)(actual_rays_per_source - 1);

  for (size_t i = 0; i < num_sources; i++) {
    float angle = i * angle_step;

    float xsource = cosf(angle);
    float ysource = sinf(angle);

    // TraceLog(LOG_INFO, "S_%d(sx,sy): (%f, %f)", i, xsource, ysource);

    for (int j = -halfNumRays; j <= halfNumRays; j++) {
      float offset = (float)j * spread_angle_step;
      float ray_angle = angle + PI + offset;
      float dx = cosf(ray_angle);
      float dy = sinf(ray_angle);

      rayset_append(&rs, (CTRay){.ox = xsource, .oy = ysource, .dx = dx, .dy = dy, .length = radius});
    }
  }

  rs.type = RAY_MODE_FAN;
  rs.metadata.fan = (RaySetFanMetadata){
      .cx = 0,
      .cy = 0,
      .radius = radius,
      .num_sources = num_sources,
      .num_rays_per_source = actual_rays_per_source,
      .angle_spread_rad = angle_spread_rad,
  };
  return rs;
}
