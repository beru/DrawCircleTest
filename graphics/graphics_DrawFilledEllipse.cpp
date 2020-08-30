
#include <vector>

#define _USE_MATH_DEFINES
#include <math.h>

#include "graphics.h"
#include "graphics_impl_common.h"

namespace Graphics {

void DrawFilledEllipse(float cx, float cy, float width, float height, pixel_t color)
{
  float a = width;
  float b = height;
  float aa = a * a;
  float bb = b * b;

  float rcp = 1.0f / (bb * aa);
  for (float y = -height; y <= height; y++) {
    float yy = y * y;
    float yyaa = yy * aa;
    for (float x = -width; x <= width; x++) {
      float xx = x * x;
      float dist = (1.0f - (xx * bb + yyaa) * rcp) * std::min(width, height) * 2.0f;
      if (dist > 1.0f)
        PutPixel(cx + x, cy + y, color);
      else if (dist < 0.0f)
        dist = 0.0f;
      else
        PutPixel(cx + x, cy + y, AdjustAlpha(color, dist));
    }
  }
}

} // namespace Graphics {

