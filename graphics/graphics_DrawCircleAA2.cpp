
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
	circleSegments.resize(tableRadius * xylen45deg + 15);
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
	CircleSegment seg,
	CircleSegment prevSeg,
	int tail
	)
{
	float areaDiff = seg.area - prevSeg.area;	// �ʐς̍���������̃��C�����̖ʐ�
	float lenDiff = prevSeg.length - seg.length;
	float curvedPart = areaDiff - seg.length;	// ��`������؂���
	float prescaledCurvedPart = curvedPart / lenDiff;

	float xMinus = cx - seg.length;
	float xPlus = cx + seg.length;
	float prevXMinus = cx - prevSeg.length;
	float prevXPlus = cx + prevSeg.length;
	
	// X���W�̐����l���قȂ�ꍇ�͉�2pixel�Ɍׂ�B
	// ���� 180���`225��
	if ((int)xMinus != (int)prevXMinus) {
		float leftArea = prescaledCurvedPart * (1.0f - frac(prevXMinus));
//		float leftArea2 = curvedPart * (ceil(prevXMinus)-prevXMinus)/lenDiff;	// �J�[�u�̈���X�P�[���������̂ŋߎ�
		float rightRectArea = 1.0f - frac(xMinus);	// �E���̃s�N�Z���̋�`��������
		float rightArea = (curvedPart - leftArea) + rightRectArea;	// 
		setPixel(xMinus-1, py, leftArea*255.0f);
		setPixel(xMinus, py, rightArea*255.0f);
	}else {
//		float remain = areaDiff - (cx - (int)(xMinus+1.0f));
		float remain = curvedPart + ceil(xMinus) - xMinus;
		setPixel(xMinus, py, remain*255.0f);
	}
	// ���Ԑ�
	DrawHorizontalLine(xMinus+1.0f, xPlus, py, color);
	// �E�� 0���` -45��
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
	float direction
	)
{
	float rr = radius * radius;
	float ratioRadius = TABLE_RADIUS / radius;
	
	int16_t py = cy;
	float fracy = frac(cy);
	float ty = 0;
	if (fracy) {
		ty += direction * (1.0-fracy) * ratioRadius;
	}
	size_t i = 1;
	if (ty < 0) {
		ty += ratioRadius;
		++i;
	}
	CircleSegment prevSeg = lerpCircleSegment(ty, radius, rr);
	size_t cnt = radius*xylen45deg;
	for (; i<=cnt+2; ++i) {
		py += direction;
		ty += ratioRadius;
		CircleSegment seg = lerpCircleSegment(ty, radius, rr);
		drawLine(py,cx,color,seg,prevSeg,0);
		prevSeg= seg;
	}
}

void drawLine2(
	int16_t x,
	int16_t x2,
	int16_t xoffset,
	float cx,
	float cy,
	pixel_t color,
	CircleSegment seg, CircleSegment prevSeg,
	int tail
	)
{
	float areaDiff = seg.area - prevSeg.area;	// �ʐς̍���������̃��C�����̖ʐ�
	float lenDiff = prevSeg.length - seg.length;
	float curvedPart = areaDiff - seg.length;	// ��`������؂���
	float prescaledCurvedPart = curvedPart / lenDiff;

	float yMinus = cy - seg.length + 1;
	float prevYMinus = cy - prevSeg.length + 1;
	float yPlus = cy + seg.length;
	float prevYPlus = cy + prevSeg.length;
	
	int iyMinus = (int)yMinus;
	int iyPlus = (int)yPlus;
#if 1
	// Y���W�̐����l���قȂ�ꍇ�͏c2pixel�Ɍׂ�B
	// �㑤
	if (iyMinus != (int)prevYMinus) {
		float leftArea = prescaledCurvedPart * (1.0f - frac(prevYMinus));
//		float leftArea2 = curvedPart * (ceil(prevYMinus)-prevYMinus)/lenDiff;	// �J�[�u�̈���X�P�[���������̂ŋߎ�
		float rightRectArea = 1.0f - frac(yMinus);	// �E���̃s�N�Z���̋�`��������
		float rightArea = (curvedPart - leftArea) + rightRectArea;	// 
		setPixel(x, iyMinus-1, leftArea*255.0f);
		setPixel(x, iyMinus, rightArea*255.0f);
		DrawHorizontalLine(x+xoffset, cx, iyMinus, color);
	}else {
//		float remain = areaDiff - (cy - (int)(yMinus+1.0f));
		float remain = curvedPart + ceil(yMinus) - yMinus;
		setPixel(x, iyMinus, remain*255.0f);
	}
#endif
	
#if 1
	// ����
	if (iyPlus != (int)prevYPlus) {
		float rightArea = prescaledCurvedPart * frac(prevYPlus);
		float leftRectArea = frac(yPlus);
		float leftArea = (curvedPart - rightArea) + leftRectArea;
		setPixel(x2, iyPlus+1, leftArea*255.0f);
		setPixel(x2, iyPlus+2, rightArea*255.0f);
		DrawHorizontalLine(cx, x2+xoffset, iyPlus+1, color);
	}else {
		float remain = areaDiff - (iyPlus - cy);
		setPixel(x2, iyPlus+1, remain*255.0f);
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
	
	float fraccx = frac(cx);
	float tx = 0;
	float tx2 = 0;
	if (fraccx) {
		tx += fraccx * ratioRadius;
		tx2 -= fraccx * ratioRadius;
	}
	size_t i = 1;
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
	for (; i<=cnt; ++i) {
		tx += ratioRadius;
		CircleSegment seg = lerpCircleSegment(tx, radius, rr);
		tx2 += ratioRadius;
		CircleSegment seg2 = lerpCircleSegment(tx2, radius, rr);
		// ��
		drawLine2(cx-i,cx-i-1,+1,cx,cy,color, seg,prevSeg,0);
		// �E
		drawLine2(cx+i-1,cx+i,0,cx,cy,color, seg2,prevSeg2,0);
		
		prevSeg = seg;
		prevSeg2 = seg2;
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
	// ����
	drawHalf(cx, cy, radius, color, 1.0);
	// �㑤
	drawHalf(cx, cy+1, radius, color, -1.0);

	drawHalf2(cx, cy, radius, color);
}

} // namespace Graphics {

