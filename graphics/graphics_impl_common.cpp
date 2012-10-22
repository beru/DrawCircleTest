
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

}