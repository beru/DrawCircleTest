
#include <vector>

#define _USE_MATH_DEFINES
#include <cmath>

#include "graphics.h"
#include "graphics_impl_common.h"

namespace Graphics {

	void DrawFilledEllipse(float cx, float cy, float width, float height, pixel_t color)
	{
		if (width <= 0 || height <= 0) {
			return;
		}
		float halfWidth = width * 0.5f;
		float halfHeight = height * 0.5f;
		int iHalfWidth = (int)((width + 0.5f) * 0.5f);
		int iHalfHeight = (int)((height + 0.5f) * 0.5f);
		float w = halfWidth;
		float h = halfHeight;
		float ww = w * w;
		float hh = h * h;
		float wwhh = ww * hh;
		float whmin = std::min(halfWidth, halfHeight);
		float intpart;
		float fracY = modf(cy, &intpart);
		float fracX = modf(cx, &intpart);
		for (int iy = -iHalfHeight; iy <= +iHalfHeight; ++iy) {
			float y = (float)iy - fracY;
			float yy = y * y;
			float yyww = yy * ww;
			float sharpness = 0.5f;
			float ratio = abs(y) / halfHeight;
			if (width > height) {
				ratio = 1.0f - ratio;
				ratio *= ratio;
				ratio *= ratio;
				ratio *= 3.0f;
				sharpness = 0.5f + ratio;
			}
			for (int ix = -iHalfWidth; ix <= +iHalfWidth; ++ix) {
				float x = (float)ix - fracX;
				if (height > width) {
					ratio = abs(x) / halfWidth;
					ratio = 1.0f - ratio;
					ratio *= ratio;
					ratio *= ratio;
					ratio *= 3.0f;
					sharpness = 0.5f + ratio;
				}
				float xx = x * x;
				float xxhh = xx * hh;
				float alpha = (1.0f - (xxhh + yyww) / wwhh) * whmin * sharpness;
				if (alpha >= 1.0f) {
					PutPixel(cx + x, cy + y, color);
				}else if (alpha > 0.0f) {
					PutPixel(cx + x, cy + y, AdjustAlpha(color, alpha));
				}
			}
		}
	}

} // namespace Graphics {

