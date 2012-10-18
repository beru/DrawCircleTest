
#include <vector>

#define _USE_MATH_DEFINES
#include <math.h>

#include "graphics.h"
#include "graphics_impl_common.h"

/*

0度から45度の位置まで1ライン毎に円の端の横位置と部分面積を記録していく。

あるラインの円の縁を描く際に、テーブルの2点間の面積の差で区間の面積が求まる。
区間の面積から長方形部分を引く事によってエッジの面積が求まる。
縁の部分のピクセル占有度を求める際に、エッジの面積と、場合によっては長方形の一部を利用する。
縁は1ピクセルだけの場合もあれば、2ピクセルに跨る場合もある。

*/

struct CircleSegment
{
	float length;	// 円の端までの距離。
	float area;		// 円の部分の面積。2点間の差で区間の面積を求められるので、エッジの面積を算出可能
};

std::vector<CircleSegment> circleSegments; // √2/2 * 256 でOK (√2 = 1.4142135623730950488016887242097)

#define RAD2DEG(a) (((a) * 180.0) / M_PI) // radian を degree に

static const float xylen45deg = sqrt(2.0)/2;	// 円45度位置の中心からの縦横長さ

static
void initTable(int tableRadius)
{
	circleSegments.resize(tableRadius * xylen45deg + 15);
	const float radius = 1.0;
	float rr = radius * radius;
	float CIRCLE_AREA = M_PI * rr;
	static const float RADIAN_CIRCUMFERENCE = 2 * M_PI;
	for (int i=0; i<circleSegments.size(); ++i) {
		float y = (float)i / tableRadius;
		float yy = y * y;
		float xx = rr - yy;
		float x = sqrt(xx); // y位置における円の接線のx位置
		float radian = asin(y/radius);
		CircleSegment& seg = circleSegments[i];
		seg.length = x;
		float sectorArea = radian / RADIAN_CIRCUMFERENCE * CIRCLE_AREA;
		float triangleArea = x*y/2;
		seg.area = sectorArea + triangleArea;
		float deg = RAD2DEG(radian);
		float hoge = 0;
	}
}

static inline
void setPixel(uint16_t x, uint16_t y, float alpha)
{
	uint8_t a = alpha;
	Graphics::putPixel(x, y, Graphics::MakePixel(0,a,0,0));
}

namespace Graphics {

static const int TABLE_RADIUS = 512;

void drawLine(
	int16_t py,
	float cx,
	pixel_t color,
	float area,
	float prevArea,
	float len,
	float prevLen,
	int tail
	)
{
	float areaDiff = area - prevArea;	// 面積の差分が今回のライン分の面積
	float lenDiff = prevLen - len;
	float curvedPart = areaDiff - len;	// 矩形部分を切り取る
	float prescaledCurvedPart = curvedPart / lenDiff;

	float xMinus = cx - len;
	float xPlus = cx + len;
	float prevXMinus = cx - prevLen;
	float prevXPlus = cx + prevLen;
	
	// X座標の整数値が異なる場合は横2pixelに跨る。
	// 左側 180°～225°
	if ((int)xMinus != (int)prevXMinus) {
		float leftArea = prescaledCurvedPart * (1.0f - frac(prevXMinus));
//		float leftArea2 = curvedPart * (ceil(prevXMinus)-prevXMinus)/lenDiff;	// カーブ領域をスケールしたもので近似
		float rightRectArea = 1.0f - frac(xMinus);	// 右側のピクセルの矩形部分だけ
		float rightArea = (curvedPart - leftArea) + rightRectArea;	// 
		setPixel(xMinus-1, py, leftArea*255.0f);
		setPixel(xMinus, py, rightArea*255.0f);
	}else {
//		float remain = areaDiff - (cx - (int)(xMinus+1.0f));
		float remain = curvedPart + ceil(xMinus) - xMinus;
		setPixel(xMinus, py, remain*255.0f);
	}
	// 中間線
	DrawHorizontalLine(xMinus+1.0f, xPlus, py, color);
	// 右側 0°～ -45°
	if ((int)xPlus != (int)prevXPlus) {
		float rightArea = prescaledCurvedPart * frac(prevXPlus);
		float leftRectArea = frac(xPlus);
		float leftArea = (curvedPart - rightArea) + leftRectArea;
		setPixel(xPlus, py, leftArea*255.0f);
		setPixel(xPlus+1, py, rightArea*255.0f);
	}else {
		float remain = areaDiff - ((int)xPlus - cx);
		setPixel(xPlus, py, remain*255.0f);
	}
}

void drawHalf(
	float cx,
	float cy,
	float radius,
	pixel_t color,
	float direction
	)
{
	float rr = radius * radius;
	float ratioRadius = TABLE_RADIUS / radius;
	
	CircleSegment cl = circleSegments[0];
	float prevLen = cl.length * radius;
	float prevArea = cl.area;
	int16_t py = cy;
	float fracy = frac(cy);
	float ty = 0;
	if (fracy) {
		ty += direction * (1.0-fracy) * ratioRadius;
	}
	size_t cnt = radius*xylen45deg;
	for (size_t i=1; i<=cnt; ++i) {
		py += direction;
		ty += ratioRadius;
		float tyFrac = frac(ty);
		CircleSegment seg0 = circleSegments[ty];
		CircleSegment seg1 = circleSegments[ty + 1];
		CircleSegment segDiff;
		segDiff.length = seg1.length - seg0.length;
		segDiff.area = seg1.area - seg0.area;
		// 描画先のピクセル境界に相当する位置の計算
		float len = (seg0.length + segDiff.length * tyFrac) * radius;
		float area = (seg0.area + segDiff.area * tyFrac) * rr;
		
		drawLine(py,cx,color,area,prevArea,len,prevLen,0);

		prevLen = len;
		prevArea = area;
	}
}

void drawLine2(
	int offset,
	float cx,
	float cy,
	pixel_t color,
	float area,
	float prevArea,
	float len,
	float prevLen,
	int tail
	)
{
	float areaDiff = area - prevArea;	// 面積の差分が今回のライン分の面積
	float lenDiff = prevLen - len;
	float curvedPart = areaDiff - len;	// 矩形部分を切り取る
	float prescaledCurvedPart = curvedPart / lenDiff;

	float yMinus = cy - len + 1;
	float prevYMinus = cy - prevLen + 1;
	float yPlus = cy + len;
	float prevYPlus = cy + prevLen;
	
#if 1
	// Y座標の整数値が異なる場合は縦2pixelに跨る。
	// 左側 180°～225°
	if ((int)yMinus != (int)prevYMinus) {
		float leftArea = prescaledCurvedPart * (1.0f - frac(prevYMinus));
//		float leftArea2 = curvedPart * (ceil(prevYMinus)-prevYMinus)/lenDiff;	// カーブ領域をスケールしたもので近似
		float rightRectArea = 1.0f - frac(yMinus);	// 右側のピクセルの矩形部分だけ
		float rightArea = (curvedPart - leftArea) + rightRectArea;	// 
		setPixel(cx+offset, yMinus-1, leftArea*255.0f);
		setPixel(cx+offset, yMinus, rightArea*255.0f);
		setPixel(cx-offset, yMinus-1, leftArea*255.0f);
		setPixel(cx-offset, yMinus, rightArea*255.0f);
		DrawHorizontalLine(cx+1-offset, cx+offset, yMinus, color);
	}else {
//		float remain = areaDiff - (cy - (int)(yMinus+1.0f));
		float remain = curvedPart + ceil(yMinus) - yMinus;
		setPixel(cx+offset, yMinus, remain*255.0f);
		setPixel(cx-offset, yMinus, remain*255.0f);
	}
#endif
	
#if 1
	// 右側 0°～ -45°
	if ((int)yPlus != (int)prevYPlus) {
		float rightArea = prescaledCurvedPart * frac(prevYPlus);
		float leftRectArea = frac(yPlus);
		float leftArea = (curvedPart - rightArea) + leftRectArea;
		setPixel(cx+offset, yPlus, leftArea*255.0f);
		setPixel(cx+offset, yPlus+1, rightArea*255.0f);
		setPixel(cx-offset, yPlus, leftArea*255.0f);
		setPixel(cx-offset, yPlus+1, rightArea*255.0f);
		DrawHorizontalLine(cx+1-offset, cx+offset, yPlus, color);
	}else {
		float remain = areaDiff - ((int)yPlus - cy);
		setPixel(cx+offset, yPlus, remain*255.0f);
		setPixel(cx-offset, yPlus, remain*255.0f);
	}
#endif
}

void drawHalf2(
	float cx,
	float cy,
	float radius,
	pixel_t color
	)
{
	float rr = radius * radius;
	float ratioRadius = TABLE_RADIUS / radius;
	
	CircleSegment cl = circleSegments[0];
	float prevLen = cl.length * radius;
	float prevArea = cl.area;
	float tx = frac(cx) * ratioRadius;
	size_t cnt = radius*xylen45deg;
	for (size_t i=1; i<=cnt; ++i) {
		tx += ratioRadius;
		float txFrac = frac(tx);
		CircleSegment seg0 = circleSegments[tx];
		CircleSegment seg1 = circleSegments[tx + 1];
		CircleSegment segDiff;
		segDiff.length = seg1.length - seg0.length;
		segDiff.area = seg1.area - seg0.area;
		// 描画先のピクセル境界に相当する位置の計算
		float len = (seg0.length + segDiff.length * txFrac) * radius;
		float area = (seg0.area + segDiff.area * txFrac) * rr;
		
		drawLine2(i,cx,cy,color,area,prevArea,len,prevLen,0);
		
		prevLen = len;
		prevArea = area;
	}
}

void DrawFilledCircleAA2(float cx, float cy, float diameter, pixel_t color)
{
	static bool inited;
	if (!inited) {
		inited = true;
		initTable(TABLE_RADIUS);
	}
	float radius = diameter / 2.0;
	// 下側
	drawHalf(cx, cy, radius, color, 1.0);
	// 上側
	drawHalf(cx, cy+1, radius, color, -1.0);

	drawHalf2(cx, cy, radius, color);
}

} // namespace Graphics {

