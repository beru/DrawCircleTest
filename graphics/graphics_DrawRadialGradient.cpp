
#include "graphics_impl_common.h"
#include <math.h>
#include <algorithm>
#include <smmintrin.h>

namespace Graphics {

inline
void drawLine_noDither(
	int16_t x, int16_t ex2, uint16_t y,
	const uint16_t* distanceTable, uint8_t shiftBits, uint8_t adjustShift2, const pixel_t* colorTable,
	uint32_t sum, int32_t d1, const uint32_t d1d
	)
{
#if 1
#if 1
	// SIMD
	__m128i* p = (__m128i*) getPixelPtr(x, y);
	uint8_t surplus = (int)p & 15;
	if (surplus) {
		surplus = (16 - surplus) / 4;
		int16_t xe = x+surplus;
		uint32_t* p2 = (uint32_t*)p;
		for (; x<xe; ++x) {
			*p2 = colorTable[sum >> shiftBits];
			++p2;
			sum += d1;
			d1 += d1d;
		}
		p = (__m128i*) ((int)p & ~15);
		++p;
	}
	__m128i vsum = _mm_setr_epi32(
		sum,
		sum + d1,
		sum + d1 + d1+d1d,
		sum + d1 + d1+d1d + d1+2*d1d);
	__m128i vd1 = _mm_setr_epi32(
		      d1 + d1+d1d + d1+2*d1d + d1+3*d1d,
		           d1+d1d + d1+2*d1d + d1+3*d1d + d1+4*d1d,
		                    d1+2*d1d + d1+3*d1d + d1+4*d1d + d1+5*d1d,
		                               d1+3*d1d + d1+4*d1d + d1+5*d1d + d1+6*d1d
		);
	__m128i vd1d = _mm_set1_epi32(4*4*d1d);
	for (; x<=ex2; x+=4) {
		__m128i vsum2 = _mm_srli_epi32(vsum, shiftBits);
		vsum = _mm_add_epi32(vsum, vd1);
		vd1 = _mm_add_epi32(vd1, vd1d);
#if 0 // SSE4
		__m128i v = _mm_unpacklo_epi64(
			_mm_unpacklo_epi32(
				_mm_castps_si128(_mm_load_ss((float*)colorTable + _mm_cvtsi128_si32(vsum2))),
				_mm_castps_si128(_mm_load_ss((float*)colorTable + _mm_extract_epi32(vsum2, 1)))),
			_mm_unpacklo_epi32(
				_mm_castps_si128(_mm_load_ss((float*)colorTable + _mm_extract_epi32(vsum2, 2))),
				_mm_castps_si128(_mm_load_ss((float*)colorTable + _mm_extract_epi32(vsum2, 3))))
		);
#else // SSE2
		__m128i p0 = _mm_castps_si128(_mm_load_ss((float*)colorTable + _mm_cvtsi128_si32(vsum2))); vsum2 = _mm_srli_si128(vsum2, 4);
		__m128i p1 = _mm_castps_si128(_mm_load_ss((float*)colorTable + _mm_cvtsi128_si32(vsum2))); vsum2 = _mm_srli_si128(vsum2, 4);
		__m128i p2 = _mm_castps_si128(_mm_load_ss((float*)colorTable + _mm_cvtsi128_si32(vsum2))); vsum2 = _mm_srli_si128(vsum2, 4);
		__m128i p3 = _mm_castps_si128(_mm_load_ss((float*)colorTable + _mm_cvtsi128_si32(vsum2)));
		__m128i v = _mm_unpacklo_epi64(_mm_unpacklo_epi32(p0, p1), _mm_unpacklo_epi32(p2, p3));
#endif
		_mm_stream_si128(p++, v);
	}
#else // Scalar
	for (; x<=ex2; ++x) {
		Graphics::pixel_t pixel = colorTable[sum >> shiftBits];
		Graphics::PutPixel(x, y, pixel);
		sum += d1;
		d1 += d1d;
	}
#endif
#else // Table Lookup 2times
	for (; x<=ex2; ++x) {
		uint32_t alpha = distanceTable[sum >> shiftBits] >> adjustShift2;
		Graphics::pixel_t pixel = colorTable[alpha];
		Graphics::PutPixel(x, y, pixel);
		sum += d1;
		d1 += d1d;
	}
#endif
}

inline void SSESqrt_Recip_Times_X( float * __restrict pOut, float * __restrict pIn )
{
   __m128 in = _mm_load_ss( pIn );
   _mm_store_ss( pOut, _mm_mul_ss( in, _mm_rsqrt_ss( in ) ) );
   // compiles to movss, movaps, rsqrtss, mulss, movss
}

void DrawRadialGradient(
	float cx, float cy, float diameter,
	const ClippingRect& clippingRect,
	const uint16_t* distanceTable, uint8_t distanceTableShifts,
	const pixel_t* colorTable,
	bool dithering
	)
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
	ey = min<float>(ey, height_);
	
	sx = max<float>(sx, clippingRect.x);
	sx = max<float>(sx, 0);

	ex = min<float>(ex, clippingRect.x+clippingRect.w);
	ex = min<float>(ex, width_);
	
	// 中心から描画領域の端までの距離の半径に対しての比率を出す。
	// 中心点からの描画領域の左端の距離と右端の距離のうち、長い方を求める。
	float dx = std::max(abs(cx - sx), abs(cx - ex));
	float dy = std::max(abs(cy - sy), abs(cy - ey));
	float lenRatio2 = radius2 / (dx*dx+dy*dy);
	uint8_t adjustShift1 = 0;
	uint8_t adjustShift2 = 0;
	if (lenRatio2 >= 8*8) {
		adjustShift1 = 8;
		adjustShift2 = 4;
	}else if (lenRatio2 >= 6*6) {
		adjustShift1 = 6;
		adjustShift2 = 3;
	}else if (lenRatio2 >= 4*4) {
		adjustShift1 = 4;
		adjustShift2 = 2;
	}else if (lenRatio2 >= 2*2) {
		adjustShift1 = 2;
		adjustShift2 = 1;
	}
	adjustShift2 += 8;
	// ordered dithering table 4x4
	uint16_t thresholds[4][4] = {
		1,9,3,11,
		13,5,15,7,
		4,12,2,10,
		16,8,14,6,
	};
	uint16_t* pt = (uint16_t*)thresholds;
	for (uint8_t i=0; i<16; ++i) {
		pt[i] <<= adjustShift2 - 3;
	}
	const uint32_t invRadius2 = (255ull << (24+adjustShift1)) / radius2;
	const uint8_t shiftBits = (28 - distanceTableShifts) + 4;
	const uint16_t mask = (1<<distanceTableShifts) - 1;
	for (uint16_t y=sy; y<=ey; ++y) {
		const float dy = cy - y;
		const float dy2 = dy * dy;
		float dx2 = radius2 - dy2;
		float dx;
//		SSESqrt_Recip_Times_X(&dx, &dx2);
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
		if (dithering) {
			const uint16_t* pThreshold = thresholds[y&3u];
			for (; x<=ex2; ++x) {
				uint32_t alpha = (distanceTable[sum >> shiftBits] + pThreshold[x&3u]) >> adjustShift2;
				Graphics::pixel_t pixel = colorTable[alpha];
				Graphics::PutPixel(x, y, pixel);
				sum += d1;
				d1 += d1d;
			}
		}else {
			drawLine_noDither(
				x, ex2, y,
				distanceTable, shiftBits, adjustShift2, colorTable,
				sum, d1, d1d);
		}
	}
}

} // namespace Graphics
