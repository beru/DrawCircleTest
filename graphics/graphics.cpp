#include "graphics_impl_common.h"

#include "sse2.h"

namespace Graphics {

pixel_t* buff_;
uint16_t width_;
uint16_t height_;
int32_t stride_;

struct Rect
{
	int16_t x;
	int16_t y;
	uint16_t w;
	uint16_t h;
};

Rect clippingRect_;

void SetCanvas(void* buff, uint16_t width, uint16_t height, int32_t stride)
{
	buff_ = (pixel_t*) buff;
	width_ = width;
	height_ = height;
	stride_ = stride;

	SetClippingRect(0,0,width,height);
}

void SetClippingRect(int16_t x, int16_t y, uint16_t w, uint16_t h)
{
	clippingRect_.x = x;
	clippingRect_.y = y;
	clippingRect_.w = w;
	clippingRect_.h = h;
	if (clippingRect_.x < 0) {
		clippingRect_.w += clippingRect_.x;
		clippingRect_.x = 0;
	}
	if (clippingRect_.h < 0) {
		clippingRect_.h += clippingRect_.y;
		clippingRect_.y = 0;
	}
}

void PutPixel(uint16_t x, uint16_t y, pixel_t color)
{
	putPixel(x, y, color);
}

void FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, pixel_t color)
{
	fillSolidRectangle(x, y, w, h, color);
}

void DrawHorizontalLine(int16_t x1, int16_t x2, int16_t y, pixel_t color)
{
	if (x1 > x2) {
		uint16_t tmp = x1;
		x1 = x2;
		x2 = tmp;
	}
	pixel_t* ptr = getPixelPtr(x1, y);
#if 0
	memset_32(ptr, color, x2-x1);
#else
	for (uint32_t x=x1; x<x2; ++x) {
		*ptr = color;
		++ptr;
	}
#endif
}

void DrawVerticalLine(int16_t x, int16_t y1, int16_t y2, pixel_t color)
{
	if (y1 > y2) {
		uint16_t tmp = y1;
		y1 = y2;
		y2 = tmp;
	}
	pixel_t* ptr = getPixelPtr(x, y1);
	pixel_t sc = to_pixel_t(color);
	for (int y=y1; y<y2; ++y) {
		*ptr = sc;
		OffsetPtr(ptr, getLineOffset());
	}
}

void DrawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, pixel_t color)
{
	if (x1 == x2) {
		DrawVerticalLine(x1, y1, y2, color);
	}else if (y1 == y2) {
		DrawHorizontalLine(x1, x2, y2, color);
	}else {
		uint16_t dx = abs(x2 - x1);
		uint16_t dy = abs(y2 - y1);
		int sx = (x1 < x2) ? 1 : -1;
		int sy = (y1 < y2) ? 1 : -1;
		pixel_t* ptr = getPixelPtr(x1, y1);
		sy *= getLineOffset();
		int dx2 = dx * 2;
		int dy2 = dy * 2;
		int E;
		pixel_t sc = to_pixel_t(color);
		// ŒX‚«‚ª1ˆÈ‰º‚Ìê‡
		if (dy <= dx) {
			E = -dx;
			for (size_t i=0; i<=dx; ++i) {
				*ptr = sc;
				ptr += sx;
				E += dy2;
				if (0 <= E) {
					OffsetPtr(ptr, sy);
					E -= dx2;
				}
			}
		// ŒX‚«‚ª1‚æ‚è‘å‚«‚¢ê‡
		}else {
			E = -dy;
			for (size_t i=0; i<=dy; ++i) {
				*ptr = sc;
				OffsetPtr(ptr, sy);
				E += dx2;
				if (0 <= E) {
					ptr += sx;
					E -= dy2;
				}
			}
		}
	}
}

} // namespace Graphics

