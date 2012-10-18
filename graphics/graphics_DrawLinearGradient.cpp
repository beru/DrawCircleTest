#include "graphics_impl_common.h"

namespace Graphics {

struct Color
{
	Color()
	{
		v[0] = 0;
		v[1] = 0;
		v[2] = 0;
		v[3] = 0;
	}
	
	Color(int v0, int v1, int v2, int v3)
	{
		v[0] = v0;
		v[1] = v1;
		v[2] = v2;
		v[3] = v3;
	}
	
	Color(uint32_t c)
	{
		v[0] = c & 0xFF;
		v[1] = c >> 8;
		v[2] = c >> 16;
		v[3] = c >> 24;
	}
	
	Color& operator -= (const Color& c) {
		v[0] -= c.v[0];
		v[1] -= c.v[1];
		v[2] -= c.v[2];
		v[3] -= c.v[3];
		return *this;
	}
	Color operator - (const Color& c) const {
		Color a = *this;
		return a -= c;
	}
	Color& operator += (const Color& c) {
		v[0] += c.v[0];
		v[1] += c.v[1];
		v[2] += c.v[2];
		v[3] += c.v[3];
		return *this;
	}
	Color operator + (const Color& c) const {
		Color a = *this;
		return a += c;
	}
	Color& operator *= (int a) {
		v[0] *= a;
		v[1] *= a;
		v[2] *= a;
		v[3] *= a;
		return *this;
	}
	Color operator * (int a) const {
		Color t = *this;
		return t *= a;
	}

	Color& operator <<= (int s) {
		v[0] <<= s;
		v[1] <<= s;
		v[2] <<= s;
		v[3] <<= s;
		return *this;
	}
	Color& operator >>= (int s) {
		v[0] >>= s;
		v[1] >>= s;
		v[2] >>= s;
		v[3] >>= s;
		return *this;
	}
	Color operator >> (int s) const {
		Color c = *this;
		return c >>= s;
	}
		
	Color& operator /= (int denominator) {
		v[0] /= denominator;
		v[1] /= denominator;
		v[2] /= denominator;
		v[3] /= denominator;
		return *this;
	}
	Color operator / (int denominator) const {
		Color c = *this;
		return c /= denominator;
	}

	operator uint32_t () {

	}

	void Clamp(int minV, int maxV)
	{
		v[0] = clamp(v[0], minV, maxV);
		v[1] = clamp(v[1], minV, maxV);
		v[2] = clamp(v[2], minV, maxV);
		v[3] = clamp(v[3], minV, maxV);
	}

	pixel_t ToPixel()
	{
		return MakePixel(v[0], v[1], v[2], v[3]);
	}
	
	int v[4];
};

template <int SHIFTNUM>
pixel_t orderedDither(uint16_t x, uint16_t y, Color c)
{

#if 1
	static const uint8_t thresholds[2][2] = {
		1,3,
		4,2,
	};
	int32_t t = thresholds[y%2][x%2] - 2;
	t <<= (SHIFTNUM - 2);
#elif 0
	static const int8_t thresholds[4][4] = {
		1,9,3,11,
		13,5,15,7,
		4,12,2,10,
		16,8,14,6,
	};
	int32_t t = thresholds[y%4][x%4] - 8;
	t <<= (SHIFTNUM - 3);
#elif 1
	static const int8_t thresholds[8][8] = {
		 1,49,13,61, 4,52,16,64,
		33,17,45,29,36,20,48,32,
		 9,57, 5,53,12,60, 8,56,
		41,25,37,21,44,28,40,24,
		 3,51,15,63, 2,50,14,62,
		35,19,47,31,34,18,46,30,
		11,59, 7,55,10,58, 6,54,
		43,27,39,23,42,26,38,22,
	};
	int32_t t = thresholds[y%8][x%8] - 32;
	t <<= (SHIFTNUM - 5);
#endif
	Color tc(t,t,t,t);
	c += tc;
	c.Clamp(0, (1<<(SHIFTNUM+8))-1);
	c >>= SHIFTNUM;
	return c.ToPixel();
}


Color tmpBuff[WIDTH];

void linearGradient_Horizontal(
	int16_t x1, pixel_t v1,
	int16_t x2, pixel_t v2,
	const ClippingRect& clippingRect,
	bool dithering
	)
{
	if (x2 < x1) {
		swap(x1, x2);
		swap(v1, v2);
	}
	Color col1(v1);
	Color col2(v2);
	col1 <<= 16;
	col2 <<= 16;
	for (uint16_t x=0; x<=x1; ++x) {
		tmpBuff[x] = col1;
	}
	
	int dx = x2 - x1;
	Color va = col2;
	Color v = col1;
	va -= v;
	va /= dx;
	for (uint16_t x=x1; x<x2; ++x) {
		tmpBuff[x] = v;
		v += va;
	}
	for (uint16_t x=x2; x<WIDTH; ++x) {
		tmpBuff[x] = col2;
	}
	
	pixel_t* pLine = getPixelPtr(0, 0);
	if (dithering) {
		for (uint16_t y=0; y<HEIGHT; ++y) {
			pixel_t* buffl = pLine;
			for (uint16_t x=0; x<WIDTH; ++x) {
				buffl[x] = orderedDither<16>(x,y,tmpBuff[x]);
			}
			OffsetPtr(pLine, getLineOffset());
		}
	}else {
		for (uint16_t y=0; y<HEIGHT; ++y) {
			pixel_t* buffl = pLine;
			for (uint16_t x=0; x<WIDTH; ++x) {
				buffl[x] = (tmpBuff[x] >> 16).ToPixel();
			}
			OffsetPtr(pLine, getLineOffset());
		}
	}
}

void linearGradient_Vertical(
	int16_t y1, pixel_t v1,
	int16_t y2, pixel_t v2,
	const ClippingRect& clippingRect,
	bool dithering
	)
{
	if (y2 < y1) {
		swap(y1, y2);
		swap(v1, v2);
	}
	pixel_t* pLine = getPixelPtr(0, 0);
	Color col1(v1);
	Color col2(v2);
	col1 <<= 16;
	col2 <<= 16;
	int dy = y2 - y1;
	Color va = col2;
	Color v = col1;
	va -= v;
	va /= dy;

	for (uint16_t y=0; y<y1; ++y) {
		for (uint16_t x=0; x<WIDTH; ++x) pLine[x] = v1;
		OffsetPtr(pLine, getLineOffset());
	}
	
	int16_t ye = min(y2, (int16_t)HEIGHT);

	if (dithering) {
		for (uint16_t y=y1; y<ye; ++y) {
			for (uint16_t x=0; x<WIDTH; ++x) {
				pLine[x] = orderedDither<16>(x,y,v);
			}
			OffsetPtr(pLine, getLineOffset());
			v += va;
		}
	}else {
		for (uint16_t y=y1; y<ye; ++y) {
			pixel_t p = (v >> 16).ToPixel();
			for (uint16_t x=0; x<WIDTH; ++x) {
				pLine[x] = p;
			}
			OffsetPtr(pLine, getLineOffset());
			v += va;
		}
	}
	
	for (uint16_t y=ye; y<HEIGHT; ++y) {
		for (uint16_t x=0; x<WIDTH; ++x) pLine[x] = v2;
		OffsetPtr(pLine, getLineOffset());
	}
}

void linearGradient(
	int16_t x1, int16_t y1, pixel_t v1,
	int16_t x2, int16_t y2, pixel_t v2,
	const ClippingRect& clippingRect,
	bool dithering
	)
{
	if (x2 < x1) {
		swap(x1, x2);
		swap(y1, y2);
		swap(v1, v2);
	}
	Color col1(v1);
	Color col2(v2);
	col1 <<= 16;
	col2 <<= 16;
	int a = x2 - x1;
	int b = y2 - y1;
	int c1 = a * x1 + b * y1;
	int c2 = a * x2 + b * y2;
	Color ov1(v1);
	Color ov2(v2);
	Color vc = ov1 * c2 - ov2 * c1;
	Color dv = ov2 - ov1;
	int dc = c2 - c1;
	int idc = 0x3FFFFF / dc;
	Color adv = dv * a;
	assert(c2 >= c1);
	int c0 = 0;
	Color bdv = dv * b;
	Color cdv = vc;
	pixel_t* pLine = getPixelPtr(0, 0);

	for (uint16_t y=0; y<HEIGHT; ++y) {
		Color cdv2 = cdv;
		uint16_t x = 0;
		int16_t x1e = min( (c1 - c0) / a, (int)WIDTH );
		int16_t x2e = min( WIDTH - (c0+a*WIDTH - c2) / a, (int)WIDTH);
		if (x1e > 0) {
			for (; x<x1e; ++x) {
				pLine[x] = v1;
			}
			cdv2 += adv * (x1e+1);
		}
		if (dithering) {
			for (; x<x2e; ++x) {
				Color n = cdv2 * idc;
				pLine[x] = orderedDither<21>(x,y,n);
				cdv2 += adv;
			}
		}else {
			for (; x<x2e; ++x) {
				Color n = cdv2 * idc;
				pLine[x] = (n >> 21).ToPixel();
				cdv2 += adv;
			}
		}
		for (; x<WIDTH; ++x) {
			pLine[x] = v2;
		}
		OffsetPtr(pLine, getLineOffset());
		c0 += b;
		cdv += bdv;
	}
}

void DrawLinearGradient(
	int16_t x1, int16_t y1, pixel_t v1,
	int16_t x2, int16_t y2, pixel_t v2,
	const ClippingRect& clippingRect,
	bool dithering
	)
{
	if (x1 == x2) {
		linearGradient_Vertical(y1, v1, y2, v2, clippingRect, dithering);
	}else if (y1 == y2) {
		linearGradient_Horizontal(x1, v1, x2, v2, clippingRect, dithering);
	}else {
		linearGradient(x1, y1, v1, x2, y2, v2, clippingRect, dithering);
	}
}

} // namespace Graphics
