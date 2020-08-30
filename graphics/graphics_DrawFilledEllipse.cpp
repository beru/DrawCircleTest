
#include <vector>

#define _USE_MATH_DEFINES
#include <math.h>

#include "graphics.h"
#include "graphics_impl_common.h"

namespace Graphics {

void DrawFilledEllipse(float cx, float cy, float width, float height, pixel_t color)
{
  float a = width - 2.f;
  float b = height - 2.f;
  float aa = a * a;
  float bb = b * b;

  float a1 = width;
  float b1 = height;
  float aa1 = a1 * a1;
  float bb1 = b1 * b1;

  float d0 = (aa * bb);
  float d1 = (aa1 * bb1);
  float rcp = 1.0f / (d1 - d0);
  float rcp2 = 1.0f + d0 * rcp;
  for (float y = -height; y <= height; y++) {
    float yy = y * y;
    float yyaa = yy * aa;
    for (float x = -width; x <= width; x++) {
#if 1
      float xx = x * x;
      float d = xx / aa + yy / bb;
      float d2 = xx / aa1 + yy / bb1;
      float dist;
      if (d2 >= 1.0f)
        dist = 0;
      else if (d <= 1.0f)
        dist = 1;
      else {
        dist = (1.0f - d2) * 50.0f;
        if (dist > 1.0f) dist = 1.0f;
      }
#else
      float xx = x * x;
      float d = (xx * bb + yyaa);
      float dist = rcp2 - d * rcp;
      if (d < d0) dist = 1.0f;
      if (d > d1) dist = 0.0f;
#endif
      PutPixel(cx + x,cy + y, color, dist);
    }
  }


}

} // namespace Graphics {

