#include "graphics_impl_common.h"
#include <math.h>
#include <algorithm>
#include <smmintrin.h>

namespace Graphics {

void DrawCircleGradient(
	float cx, float cy, float diameter1, float diameter2,
	const ClippingRect& clippingRect,
	const uint16_t* distanceTable, uint8_t distanceTableShifts,
	const pixel_t* colorTable,
	bool dithering)
{
	if (diameter < 2) {
		return;
	}
	const float radius = diameter / 2;
	const float radius2 = (diameter * diameter) / 4;
	float sy = cy - radius;
	float ey = sy + diameter;
	float sx = cx - radius;
	float ex = sx + diameter;

	sy = max<float>(sy, clippingRect.y);
	sy = max<float>(sy, 0);
	
	ey = min<float>(ey, clippingRect.y+clippingRect.h);
	ey = min<float>(ey, HEIGHT);
	
	sx = max<float>(sx, clippingRect.x);
	sx = max<float>(sx, 0);

	ex = min<float>(ex, clippingRect.x+clippingRect.w);
	ex = min<float>(ex, WIDTH);
	
	// 中心から描画領域の端までの距離の半径に対しての比率を出す。
	// 中心点からの描画領域の左端の距離と右端の距離のうち、長い方を求める。
	float dx = std::max(abs(cx - sx), abs(cx - ex));
	float dy = std::max(abs(cy - sy), abs(cy - ey));
	float lenRatio2 = radius2 / (dx*dx+dy*dy);
	
	// ordered dithering table 4x4
	uint16_t thresholds[4][4] = {
		1,9,3,11,
		13,5,15,7,
		4,12,2,10,
		16,8,14,6,
	};
	const uint32_t invRadius2 = (255ull << 24) / radius2;
	const uint8_t shiftBits = (28 - distanceTableShifts) + 4;
	const uint16_t mask = (1<<distanceTableShifts) - 1;
	for (uint16_t y=sy; y<=ey; ++y) {
		const float dy = cy - y;
		const float dy2 = dy * dy;
		float dx2 = radius2 - dy2;
		float dx;
		dx = sqrt(dx2);
		const int16_t sx2 = max(sx, cx-dx) + 1;
		const int16_t ex2 = min(ex, cx+dx) - 1;
		
		dx = cx - sx2;
		dx2 = dx * dx;
		
		uint32_t sum = (dy2 + dx2) * invRadius2;
		int32_t d1 = (dx * -2 + 1) * invRadius2;
		uint32_t d1d = invRadius2 * 2;
		
		// TODO: 縁の描画はピクセル占有率等も考慮して行う
		int16_t x = sx2;
		const uint16_t* pThreshold = thresholds[y&3u];
		for (; x<=ex2; ++x) {
			uint32_t alpha = (distanceTable[sum >> shiftBits] + pThreshold[x&3u]) >> adjustShift2;
			Graphics::pixel_t pixel = colorTable[alpha];
			Graphics::PutPixel(x, y, pixel);
			sum += d1;
			d1 += d1d;
		}
	}
}


} // namespace Graphics
