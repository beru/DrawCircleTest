#include "graphics_impl_common.h"

#include <algorithm>

namespace Graphics {

pixel_t* buff_;
uint16_t width_;
uint16_t height_;
int32_t stride_;

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

static
void inline PutPixel_Unsafe(pixel_t* pDest, pixel_t color)
{
	*pDest = BlendColor(color, *pDest);
}

void PutPixel(int16_t x, int16_t y, pixel_t color)
{
	if (RectContains(clippingRect_, x, y)) {
		pixel_t* pDest = GetPixelPtr(x, y);
		PutPixel_Unsafe(pDest, color);
	}
}

pixel_t MakePixel(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	return (a<<24) | (b<<16) | (g<<8) | r;
}

pixel_t ReplaceAlpha(pixel_t color, uint8_t a)
{
	return (color & ~AMASK) | (a<<ASHIFT);
}

pixel_t AdjustAlpha(pixel_t color, float v)
{
	return (color & ~AMASK) | ((uint32_t)(color * v) & AMASK);
}

// http://www.virtualdub.org/blog/pivot/entry.php?id=117
pixel_t BlendColor(pixel_t foreColor, pixel_t backColor)
{
	assert(sizeof(pixel_t) == sizeof(uint32_t));
	uint32_t sRB = foreColor & 0xff00ff;
	uint32_t sG = foreColor & 0x00ff00;
	uint32_t dRB = backColor & 0xff00ff;
	uint32_t dG = backColor & 0x00ff00;
	
	uint32_t sA = (foreColor & 0xff000000) >> 24;
	sA += (sA > 0);
	uint32_t oRB = (dRB + (((sRB - dRB) * sA + 0x800080) >> 8)) & 0xff00ff;
	uint32_t oG = (dG + (((sG - dG ) * sA + 0x008000) >> 8)) & 0x00ff00;
	pixel_t ret = oRB + oG;
	return ret;
}

void FillRect(int16_t x, int16_t y, uint16_t w, uint16_t h, pixel_t color)
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
	
	pixel_t* ptr = GetPixelPtr(x2, y2);
	for (int iy=0; iy<h2; ++iy) {
		for (uint16_t ix=0; ix<w2; ++ix) {
			PutPixel_Unsafe(ptr+ix, color);
		}
		OffsetPtr(ptr, GetLineOffset());
	}
}

void DrawHorizontalLine(int16_t x1, int16_t x2, int16_t y, pixel_t color)
{
	if (y < clippingRect_.y || y >= clippingRect_.y+clippingRect_.h) {
		return;
	}
	if (x1 > x2) {
		uint16_t tmp = x1;
		x1 = x2;
		x2 = tmp;
	}
	x1 = std::max<int16_t>(x1, clippingRect_.x);
	x1 = std::min<int16_t>(x1, clippingRect_.x+clippingRect_.w);
	x2 = std::max<int16_t>(x2, clippingRect_.x);
	x2 = std::min<int16_t>(x2, clippingRect_.x+clippingRect_.w);
	
	pixel_t* ptr = GetPixelPtr(x1, y);
	for (uint32_t x=x1; x<x2; ++x) {
		PutPixel_Unsafe(ptr, color);
		++ptr;
	}
}

void DrawVerticalLine(int16_t x, int16_t y1, int16_t y2, pixel_t color)
{
	if (y1 > y2) {
		uint16_t tmp = y1;
		y1 = y2;
		y2 = tmp;
	}
	pixel_t* ptr = GetPixelPtr(x, y1);
	for (int y=y1; y<y2; ++y) {
		*ptr = color;
		OffsetPtr(ptr, GetLineOffset());
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
		pixel_t* ptr = GetPixelPtr(x1, y1);
		sy *= GetLineOffset();
		int dx2 = dx * 2;
		int dy2 = dy * 2;
		int E;
		// 傾きが1以下の場合
		if (dy <= dx) {
			E = -dx;
			for (size_t i=0; i<=dx; ++i) {
				*ptr = color;
				ptr += sx;
				E += dy2;
				if (0 <= E) {
					OffsetPtr(ptr, sy);
					E -= dx2;
				}
			}
		// 傾きが1より大きい場合
		}else {
			E = -dy;
			for (size_t i=0; i<=dy; ++i) {
				*ptr = color;
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

