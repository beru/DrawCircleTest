#ifndef GRAPHICS_IMPL_COMMON_H_INCLUDED__
#define GRAPHICS_IMPL_COMMON_H_INCLUDED__

#include "graphics.h"

namespace Graphics {

struct Rect
{
	int16_t x;
	int16_t y;
	uint16_t w;
	uint16_t h;
};

extern pixel_t* buff_;
extern uint16_t width_;
extern uint16_t height_;
extern int32_t stride_;

extern Rect clippingRect_;

bool RectIntersect(const Rect& a, const Rect& b, Rect& c);

bool RectContains(const Rect& r, int16_t x, int16_t y);

static inline
int getLineOffset()
{
	return stride_;
}

static inline
pixel_t* getPixelPtr(uint16_t x, uint16_t y)
{
	pixel_t* ptr = buff_ + x;
	OffsetPtr(ptr, y*stride_);
	return ptr;
}

static inline
void putPixel(uint16_t x, uint16_t y, pixel_t color)
{
	if (RectContains(clippingRect_, x, y)) {
		*getPixelPtr(x, y) = color;
	}
}

template <typename T>
static inline
void swap(T& a, T& b)
{
	T tmp = a;
	a = b;
	b = tmp;
}

template <typename T>
static inline
T min(T a, T b)
{
	return a<b ? a : b;
}

template <typename T>
static inline
T min(T a, T b, T c)
{
	return min(min(a,b),c);
}

#if 0
static inline
int min(int x, int y)
{
	assert(INT_MIN <= (x - y));
	assert((x - y) <= INT_MAX);
	return y + ((x - y) & ((x - y) >> (sizeof(int) * CHAR_BIT - 1))); // min(x, y)
}

static inline
int max(int x, int y)
{
	assert(INT_MIN <= (x - y));
	assert((x - y) <= INT_MAX);
	return x - ((x - y) & ((x - y) >> (sizeof(int) * CHAR_BIT - 1))); // max(x, y)
}
#endif

template <typename T>
static inline
T max(T a, T b)
{
	return a>b ? a : b;
}

template <typename T>
static inline
T max(T a, T b, T c)
{
	return max(max(a,b),c);
}

template <typename T>
static inline
T clamp(T v, T minV, T maxV)
{
	return min(max(v,minV), maxV);
}

template <typename T>
static inline
T abs(T a)
{
	return a<0 ? -a:a;
}

inline
double frac(double v)
{
	return v - (int)v;
}

// http://www.lomont.org/Math/Papers/2003/InvSqrt.pdf
static inline
float InvSqrt(float x)
{
	float xhalf = 0.5f*x;
	int i = *(int*)&x; // get bits for floating value
	i = 0x5f375a86- (i>>1); // gives initial guess y0
	x = *(float*)&i; // convert bits back to float
	x = x*(1.5f-xhalf*x*x); // Newton step, repeating increases accuracy
	return x;
}

void fillSolidRectangle(int16_t x, int16_t y, uint16_t w, uint16_t h, pixel_t color);

}

#endif // #ifndef GRAPHICS_IMPL_COMMON_H_INCLUDED__
