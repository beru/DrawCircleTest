
#include <vector>

#define _USE_MATH_DEFINES
#include <math.h>

#include "graphics.h"
#include "graphics_impl_common.h"

namespace Graphics {

	void DrawFilledEllipse(float cx, float cy, float width, float height, pixel_t color)
	{
		if (width == 0 || height == 0) {
			return;
		}
		float halfWidth = width / 2.0f;
		float halfHeight = height / 2.0f;
		float w = halfWidth;
		float h = halfHeight;
		float ww = w * w;
		float hh = h * h;
		float wwhh = ww * hh;
		float whmin = std::min(halfWidth, halfHeight);
		for (float y = -halfHeight; y <= +halfHeight; y++) {
			float yy = y * y;
			float yyww = yy * ww;
			float sharpness = 0.5f;
			float ratio = abs(y) / halfHeight;
			if (width > height) {
				ratio = 1.0f - ratio;
				ratio *= ratio;
				ratio *= ratio;
				ratio *= 3.0f;
				sharpness = 0.5 + ratio;
			}
			for (float x = -halfWidth; x <= +halfWidth; x++) {
				if (height > width) {
					ratio = abs(x) / halfWidth;
					ratio = 1.0f - ratio;
					ratio *= ratio;
					ratio *= ratio;
					ratio *= 3.0f;
					sharpness = 0.5 + ratio;
				}
				float xx = x * x;
				float xxhh = xx * hh;
				float dist = (1.0f - (xxhh + yyww) / wwhh) * whmin * sharpness;
				if (dist >= 1.0f) {
					PutPixel(cx + x, cy + y, color);
				}else if (dist <= 0.0f) {
					dist = 0.0f;
				}else {
					PutPixel(cx + x, cy + y, AdjustAlpha(color, dist));
				}
			}
		}
	}

} // namespace Graphics {

