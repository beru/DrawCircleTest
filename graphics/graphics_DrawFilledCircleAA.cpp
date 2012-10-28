
#include <vector>

#define _USE_MATH_DEFINES
#include <math.h>

#include "graphics.h"
#include "graphics_impl_common.h"

/*

0�x����45�x�̈ʒu�܂�1���C�����ɉ~�̒[�̉��ʒu�ƕ����ʐς��L�^���Ă����B

���郉�C���̉~�̉���`���ۂɁA�e�[�u����2�_�Ԃ̖ʐς̍��ŋ�Ԃ̖ʐς����܂�B
��Ԃ̖ʐς��璷���`�������������ɂ���ăG�b�W�̖ʐς����܂�B
���̕����̃s�N�Z����L�x�����߂�ۂɁA�G�b�W�̖ʐςƁA�ꍇ�ɂ���Ă͒����`�̈ꕔ�𗘗p����B
����1�s�N�Z�������̏ꍇ������΁A2�s�N�Z���Ɍׂ�ꍇ������B

*/

struct CircleSegment
{
	float length;	// �~�̒[�܂ł̋����B
	float area;		// �~�̕����̖ʐρB2�_�Ԃ̍��ŋ�Ԃ̖ʐς����߂���̂ŁA�G�b�W�̖ʐς��Z�o�\
};

std::vector<CircleSegment> circleSegments; // ��2/2 * 256 ��OK (��2 = 1.4142135623730950488016887242097)

#define RAD2DEG(a) (((a) * 180.0) / M_PI) // radian �� degree ��

static const float xylen45deg = sqrt(2.0)/2;	// �~45�x�ʒu�̒��S����̏c������

static
void initTable(int tableRadius)
{
	circleSegments.resize(tableRadius * xylen45deg * 2);
	const float radius = 1.0;
	float rr = radius * radius;
	float CIRCLE_AREA = M_PI * rr;
	static const float RADIAN_CIRCUMFERENCE = 2 * M_PI;
	for (int i=0; i<circleSegments.size(); ++i) {
		float y = (float)i / tableRadius;
		float yy = y * y;
		float xx = rr - yy;
		float x = sqrt(xx); // y�ʒu�ɂ�����~�̐ڐ���x�ʒu
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

namespace Graphics {

static const int TABLE_RADIUS = 512;

static inline
void setPixel(int16_t x, int16_t y, pixel_t color, float alpha)
{
	PutPixel(x, y, color, alpha);
}

static inline
float distCeil(float v)
{
	return ceil(v) - v;
}

static inline
float distFloor(float v)
{
	return v - floor(v);
}

void drawLine(
	int16_t py,
	float cx,
	pixel_t color,
	float alpha,
	CircleSegment seg,
	CircleSegment prevSeg,
	int tail
	)
{
	float xMinus = cx - seg.length;
	float xPlus = cx + seg.length;
	int16_t ixMinus = floor(xMinus);
	int16_t ixPlus = floor(xPlus);
	
	float prevXMinus = cx - prevSeg.length;
	float prevXPlus = cx + prevSeg.length;
	
	float areaDiff = seg.area - prevSeg.area;	// �ʐς̍���������̃��C�����̖ʐ�
	float lenDiff = prevSeg.length - seg.length;
	float curvedPart = areaDiff - seg.length;	// ��`������؂���
	float prescaledCurvedPart = curvedPart / lenDiff;

	// X���W�̐����l���قȂ�ꍇ�͉�2pixel�Ɍׂ�B
	// ���� 180���`225��
	if (ixMinus != (int)prevXMinus) {
		float leftArea = prescaledCurvedPart * distCeil(prevXMinus);
//		float leftArea2 = curvedPart * (ceil(prevXMinus)-prevXMinus)/lenDiff;	// �J�[�u�̈���X�P�[���������̂ŋߎ�
		float rightRectArea = distCeil(xMinus);	// �E���̃s�N�Z���̋�`��������
		float rightArea = (curvedPart - leftArea) + rightRectArea;	// 
		setPixel(ixMinus-1, py, color, leftArea*alpha);
		setPixel(ixMinus, py, color, rightArea*alpha);
	}else {
//		float remain = areaDiff - (cx - (int)(xMinus+1.0f));
		float remain = curvedPart + ceil(xMinus) - xMinus;
		setPixel(ixMinus, py, color, remain*alpha);
	}
	// ���Ԑ�
	DrawHorizontalLine(ixMinus+1, ixPlus, py, color);
	// �E�� 0���` -45��
	if (ixPlus != (int)prevXPlus) {
		float rightArea = prescaledCurvedPart * distFloor(prevXPlus);
		float leftRectArea = distFloor(xPlus);
		float leftArea = (curvedPart - rightArea) + leftRectArea;
		setPixel(ixPlus, py, color, leftArea*alpha);
		setPixel(ixPlus+1, py, color, rightArea*alpha);
	}else {
		float remain = areaDiff - (ixPlus - cx);
		setPixel(ixPlus, py, color, remain*alpha);
	}
}

// �`���̃s�N�Z�����E�ɑ�������ʒu�Ɩʐς̌v�Z
CircleSegment lerpCircleSegment(float pos, float radius, float rr)
{
	CircleSegment seg0 = circleSegments[pos];
	CircleSegment seg1 = circleSegments[pos + 1];
	CircleSegment diff;
	diff.length = seg1.length - seg0.length;
	diff.area = seg1.area - seg0.area;
	
	float fraction = frac(pos);
	CircleSegment ret;
	ret.length = (seg0.length + diff.length * fraction) * radius;
	ret.area = (seg0.area + diff.area * fraction) * rr;
	return ret;
}

void drawHalf(
	float cx,
	float cy,
	float radius,
	pixel_t color,
	float alpha,
	float direction,
	int16_t& lasty
	)
{
	float rr = radius * radius;
	float ratioRadius = TABLE_RADIUS / radius;
	
	int16_t py = cy;
	float distcy = distFloor(cy);
	float ty = 0;
	if (distcy) {
		ty += direction * (1.0-distcy) * ratioRadius;
	}
	size_t i = 1;
	if (ty < 0) {
		ty += ratioRadius;
		++i;
	}
	CircleSegment prevSeg = lerpCircleSegment(ty, radius, rr);
	size_t cnt = radius*xylen45deg;
	for (; i<cnt+3; ++i) {
		py += direction;
		ty += ratioRadius;
		CircleSegment seg = lerpCircleSegment(ty, radius, rr);
		int16_t ipy = (int16_t)py;
		if (ipy >= clippingRect_.y && ipy < clippingRect_.y+clippingRect_.h) {
			drawLine(ipy,cx,color,alpha,seg,prevSeg,0);
			lasty = ipy;
		}
		prevSeg = seg;
	}
}

void drawLine2_Top(
	int16_t x,
	int16_t xoffset,
	int16_t yoffset,
	float cx,
	float cy,
	pixel_t color,
	float alpha,
	CircleSegment seg, CircleSegment prevSeg,
	int16_t ylimit
	)
{
	float yMinus = cy - seg.length;
	int iyMinus = floor(yMinus);
	// �㑤
	// Y���W�̐����l���قȂ�ꍇ�͏c2pixel�Ɍׂ�B
	int16_t y = iyMinus + yoffset;
	if (y < ylimit) {
		float prevYMinus = cy - prevSeg.length;
		float areaDiff = seg.area - prevSeg.area;	// �ʐς̍���������̃��C�����̖ʐ�
		float curvedPart = areaDiff - seg.length;	// ��`������؂���
		if (iyMinus != (int)floor(prevYMinus)) {
			float lenDiff = prevSeg.length - seg.length;
			float prescaledCurvedPart = curvedPart / lenDiff;
			float leftArea = prescaledCurvedPart * distCeil(prevYMinus);
	//		float leftArea2 = curvedPart * (ceil(prevYMinus)-prevYMinus)/lenDiff;	// �J�[�u�̈���X�P�[���������̂ŋߎ�
			float rightRectArea = distCeil(yMinus);	// �E���̃s�N�Z���̋�`��������
			float rightArea = (curvedPart - leftArea) + rightRectArea;	// 
			setPixel(x, y-1, color, leftArea*alpha);
			setPixel(x, y, color, rightArea*alpha);
			DrawHorizontalLine(x+xoffset, cx, y, color);
		}else {
	//		float remain = areaDiff - (cy - (int)(yMinus+1.0f));
			float remain = curvedPart + ceil(yMinus) - yMinus;
			setPixel(x, y, color, remain*alpha);
		}
	}
	
}

void drawLine2_Bottom(
	int16_t x,
	int16_t xoffset,
	int16_t yoffset,
	float cx,
	float cy,
	pixel_t color,
	float alpha,
	CircleSegment seg, CircleSegment prevSeg,
	int16_t ylimit
	)
{
	float yPlus = cy + seg.length;
	int iyPlus = floor(yPlus);
#if 1
	// ����
	int16_t y = iyPlus+yoffset;
	if (y > ylimit) {
		float areaDiff = seg.area - prevSeg.area;	// �ʐς̍���������̃��C�����̖ʐ�
		float prevYPlus = cy + prevSeg.length;
		if (iyPlus != (int)prevYPlus) {
			float lenDiff = prevSeg.length - seg.length;
			float curvedPart = areaDiff - seg.length;	// ��`������؂���
			float prescaledCurvedPart = curvedPart / lenDiff;
			float rightArea = prescaledCurvedPart * distFloor(prevYPlus);
			float leftRectArea = distFloor(yPlus);
			float leftArea = (curvedPart - rightArea) + leftRectArea;
			setPixel(x, y, color, leftArea*alpha);
			DrawHorizontalLine(cx, x+xoffset, y, color);
			setPixel(x, y+1, color, rightArea*alpha);
		}else {
			float remain = areaDiff - (iyPlus - cy);
			setPixel(x, y, color, remain*alpha);
		}
	}
#endif
}

void drawHalf2(
	float cx,
	float cy,
	float radius,
	pixel_t color,
	float alpha,
	int16_t ylimits[2]
	)
{
	float rr = radius * radius;
	float ratioRadius = TABLE_RADIUS / radius;
	
	float distcx = distFloor(cx);
	float tx = 0;
	float tx2 = 0;
	if (distcx) {
		tx += distcx * ratioRadius;
		tx2 -= distcx * ratioRadius;
	}
	size_t cnt = radius*xylen45deg;
	if (tx2 < 0) {
		tx += ratioRadius;
		tx2 += ratioRadius;
	}
	CircleSegment prevSeg = lerpCircleSegment(tx, radius, rr);
	CircleSegment prevSeg2 = lerpCircleSegment(tx2, radius, rr);
	if (frac(cy)) {
		cy -= 1.0;
	}
	float tx_ = tx;
	float tx2_ = tx2;
	CircleSegment prevSeg_ = prevSeg;
	CircleSegment prevSeg2_ = prevSeg2;

	int yOffset1 = 1;
	if (frac(cx)) yOffset1 -= 1;
	if (frac(cy)) yOffset1 += 1;
	// ��
	for (size_t i=1; i<cnt+3; ++i) {
		tx += ratioRadius;
		CircleSegment seg = lerpCircleSegment(tx, radius, rr);
		tx2 += ratioRadius;
		CircleSegment seg2 = lerpCircleSegment(tx2, radius, rr);
		// ��
		drawLine2_Top(cx-i,+1,yOffset1,cx,cy,color,alpha, seg,prevSeg,ylimits[0]);
		// �E
		drawLine2_Top(cx+i-1,0,yOffset1,cx,cy,color,alpha, seg2,prevSeg2,ylimits[0]);
		
		prevSeg = seg;
		prevSeg2 = seg2;
	}
	tx = tx_;
	tx2 = tx2_;
	prevSeg = prevSeg_;
	prevSeg2 = prevSeg2_;
	int yOffset2 = 0;
	if (frac(cx)) yOffset2 += 1;
	// �����̍ŏ���2pixel
	{
		// ��
		CircleSegment seg = lerpCircleSegment(tx+ratioRadius, radius, rr);
		if ((int)(cy+seg.length)==(int)(cy+prevSeg.length)) {
			drawLine2_Bottom(cx-1,+1,yOffset2,cx,cy,color,alpha, seg,prevSeg,ylimits[1]);
		}
		// �E
		CircleSegment seg2 = lerpCircleSegment(tx2+ratioRadius, radius, rr);
		if ((int)(cy+seg2.length)==(int)(cy+prevSeg2.length)) {
			drawLine2_Bottom(cx+1-1,0,yOffset2,cx,cy,color,alpha, seg2,prevSeg2,ylimits[1]);
		}
	}
	// ��
	for (size_t i=1; i<cnt+3; ++i) {
		tx += ratioRadius;
		CircleSegment seg = lerpCircleSegment(tx, radius, rr);
		tx2 += ratioRadius;
		CircleSegment seg2 = lerpCircleSegment(tx2, radius, rr);
		// ��
		drawLine2_Bottom(cx-i-1,+1,yOffset2,cx,cy,color,alpha, seg,prevSeg,ylimits[1]);
		// �E
		drawLine2_Bottom(cx+i,0,yOffset2,cx,cy,color,alpha, seg2,prevSeg2,ylimits[1]);
		
		prevSeg = seg;
		prevSeg2 = seg2;
	}
}

void DrawFilledCircleAA(float cx, float cy, float diameter, pixel_t color)
{
	static bool inited;
	if (!inited) {
		inited = true;
		initTable(TABLE_RADIUS);
	}

	if (color & AMASK) {
		float radius = diameter / 2.0;
		
		Rect recCircle;
		recCircle.x = cx - radius;
		recCircle.y = cy - radius;
		recCircle.w = diameter;
		recCircle.h = diameter;
		Rect irec;
		if (!RectIntersect(recCircle, clippingRect_, irec)) {
			return;
		}
		
		float alpha = (color >> ASHIFT) / 255.0;
		int16_t ys[2];
		// �㑤
		drawHalf(cx, cy+1, radius, color, alpha, -1.0, ys[0]);
		// ����
		drawHalf(cx, cy, radius, color, alpha, 1.0, ys[1]);
		
		drawHalf2(cx, cy, radius, color, alpha, ys);
	}
}

} // namespace Graphics {

