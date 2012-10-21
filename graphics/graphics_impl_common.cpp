
#include "graphics_impl_common.h"

namespace Graphics {

bool RectIntersect(const Rect& a, const Rect& b, Rect& c)
{
	int16_t minX = max(a.x, b.x);
	int16_t maxX = min(a.x+a.w, b.x+b.w);
	int16_t minY = max(a.y, b.y);
	int16_t maxY = min(a.y+a.h, b.y+b.h);
	
	c.x = minX;
	c.w = (maxX > minX) ? (maxX - minX) : 0;
	c.y = minY;
	c.h = (maxY > minY) ? (maxY - minY) : 0;
	return c.w && c.h;
}

bool RectContains(const Rect& r, int16_t x, int16_t y)
{
	return
		r.x <= x
		&& x < (r.x + r.w)
		&& r.y <= y
		&& y < (r.y + r.h)
	;
}

// 単色塗りつぶし処理
void fillSolidRectangle(int16_t x, int16_t y, uint16_t w, uint16_t h, uint32_t color)
{
	// 領域チェック
	if (w == 0 || h == 0) {
		return;
	}
	int16_t x2 = x;
	int16_t y2 = y;
	uint16_t w2 = w;
	uint16_t h2 = h;
	// 表示範囲外チェック
	if (x < 0) {
		if (-x >= w) {
			return;
		}
		x2 = 0;
		w2 += x;
	}
	if (y < 0) {
		if (-y >= h) {
			return;
		}
		y2 = 0;
		h2 += y;
	}

	if (x2 >= width_ || y2>= height_) {
		return;
	}
	if (x2 + w2 >= width_) {
		w2 = width_ - x2;
	}
	if (y2 + h2 >= height_) {
		h2 = height_ - y2;
	}
	
	pixel_t* ptr = getPixelPtr(x2, y2);
	for (int iy=0; iy<h2; ++iy) {
		for (uint16_t ix=0; ix<w2; ++ix) {
			ptr[ix] = color;
		}
		OffsetPtr(ptr, getLineOffset());
	}
}

}