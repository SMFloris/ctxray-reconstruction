#ifndef UTILS_H
#define UTILS_H

#include "raylib.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  float xmin, ymin, xmax, ymax;
} Rect;

typedef struct {
  bool intersects;
  float length;
  float t1, t2;
} LiangBarskyResult;

// Load a P5 (binary) PGM image
Image LoadPGM(const char *path, unsigned char **output_rawPixels) {
  FILE *f = fopen(path, "rb");
  if (!f) {
    TraceLog(LOG_ERROR, "Cannot open PGM: %s", path);
    return (Image){0};
  }

  char magic[3] = {0};
  if (fscanf(f, "%2s", magic) != 1 || magic[0] != 'P' || magic[1] != '5') {
    TraceLog(LOG_ERROR, "Not a valid P5 PGM file");
    fclose(f);
    return (Image){0};
  }

  int w, h, maxval;
  if (fscanf(f, "%d %d %d", &w, &h, &maxval) != 3) {
    TraceLog(LOG_ERROR, "Failed to read PGM header");
    fclose(f);
    return (Image){0};
  }
  fgetc(f);

  size_t size = (size_t)w * h;
  unsigned char *pixels = (unsigned char *)malloc(size);
  if (!pixels || fread(pixels, 1, size, f) != size) {
    TraceLog(LOG_ERROR, "Failed to read PGM pixel data");
    free(pixels);
    fclose(f);
    return (Image){0};
  }

  // Make a copy for the raw grayscale data before ImageFormat modifies/frees it
  unsigned char *rawCopy = (unsigned char *)malloc(size);
  memcpy(rawCopy, pixels, size);
  *output_rawPixels = rawCopy;
  fclose(f);

  Image img = {
      .data = pixels,
      .width = w,
      .height = h,
      .mipmaps = 1,
      .format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE,
  };
  ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
  return img;
}

/**
 * Liang-Barsky ray-rectangle intersection
 *
 * Ray: P(t) = (ox, oy) + t * (dx, dy), t >= 0
 * Returns intersection result with t1, t2 parameters and segment length.
 *
 * See Wikipedia algorithm:
 * https://en.wikipedia.org/wiki/Liang%E2%80%93Barsky_algorithm
 */
LiangBarskyResult liang_barsky_ray(const Rect *r, float ox, float oy, float dx, float dy) {
  // p: negated direction components, q: distance to edges
  // p1 = -dx, p2 = dx, p3 = -dy, p4 = dy
  // q1 = ox - xmin, q2 = xmax - ox, q3 = oy - ymin, q4 = ymax - oy
  float p1 = -dx, p2 = dx, p3 = -dy, p4 = dy;
  float q1 = ox - r->xmin, q2 = r->xmax - ox, q3 = oy - r->ymin, q4 = r->ymax - oy;

  // Check for parallel lines outside the clipping window
  if ((p1 == 0 && q1 < 0) || (p2 == 0 && q2 < 0) ||
      (p3 == 0 && q3 < 0) || (p4 == 0 && q4 < 0)) {
    return (LiangBarskyResult){.intersects = false};
  }

  // Entry and exit parameter arrays
  float entryParams[5], exitParams[5];
  int entryIdx = 1, exitIdx = 1;
  entryParams[0] = 0.0f;
  exitParams[0] = INFINITY;

  // Process horizontal edges (left/right)
  if (p1 != 0) {
    float r1 = q1 / p1;
    float r2 = q2 / p2;
    if (p1 < 0) {
      entryParams[entryIdx++] = r1; // entering from left
      exitParams[exitIdx++] = r2;   // exiting from right
    } else {
      entryParams[entryIdx++] = r2;
      exitParams[exitIdx++] = r1;
    }
  }

  // Process vertical edges (bottom/top)
  if (p3 != 0) {
    float r3 = q3 / p3;
    float r4 = q4 / p4;
    if (p3 < 0) {
      entryParams[entryIdx++] = r3;
      exitParams[exitIdx++] = r4;
    } else {
      entryParams[entryIdx++] = r4;
      exitParams[exitIdx++] = r3;
    }
  }

  // u1 = maximum of entry parameters
  float u1 = entryParams[0];
  for (int i = 1; i < entryIdx; i++)
    if (entryParams[i] > u1)
      u1 = entryParams[i];

  // u2 = minimum of exit parameters
  float u2 = exitParams[0];
  for (int i = 1; i < exitIdx; i++)
    if (exitParams[i] < u2)
      u2 = exitParams[i];

  // Check if valid intersection exists
  if (u1 > u2 || u2 < 0.0f) {
    return (LiangBarskyResult){.intersects = false};
  }

  // Clamp u1 to ray start (t >= 0)
  if (u1 < 0.0f)
    u1 = 0.0f;

  float len = (u2 - u1) * sqrtf(dx * dx + dy * dy);
  return (LiangBarskyResult){.intersects = true, .length = len, .t1 = u1, .t2 = u2};
}

#endif
