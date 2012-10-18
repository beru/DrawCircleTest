
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
	float area,
	float prevArea,
	float len,
	float prevLen,
	int tail
	)
{
	float areaDiff = area - prevArea;	// �ʐς̍���������̃��C�����̖ʐ�
	float lenDiff = prevLen - len;
	float curvedPart = areaDiff - len;	// ��`������؂���
	float prescaledCurvedPart = curvedPart / lenDiff;

	float xMinus = cx - len;
	float xPlus = cx + len;
	float prevXMinus = cx - prevLen;
	float prevXPlus = cx + prevLen;
	
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
		// �`���̃s�N�Z�����E�ɑ�������ʒu�̌v�Z
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
	float areaDiff = area - prevArea;	// �ʐς̍���������̃��C�����̖ʐ�
	float lenDiff = prevLen - len;
	float curvedPart = areaDiff - len;	// ��`������؂���
	float prescaledCurvedPart = curvedPart / lenDiff;

	float yMinus = cy - len + 1;
	float prevYMinus = cy - prevLen + 1;
	float yPlus = cy + len;
	float prevYPlus = cy + prevLen;
	
#if 1
	// Y���W�̐����l���قȂ�ꍇ�͏c2pixel�Ɍׂ�B
	// ���� 180���`225��
	if ((int)yMinus != (int)prevYMinus) {
		float leftArea = prescaledCurvedPart * (1.0f - frac(prevYMinus));
//		float leftArea2 = curvedPart * (ceil(prevYMinus)-prevYMinus)/lenDiff;	// �J�[�u�̈���X�P�[���������̂ŋߎ�
		float rightRectArea = 1.0f - frac(yMinus);	// �E���̃s�N�Z���̋�`��������
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
	// �E�� 0���` -45��
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
		// �`���̃s�N�Z�����E�ɑ�������ʒu�̌v�Z
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
	// ����
	drawHalf(cx, cy, radius, color, 1.0);
	// �㑤
	drawHalf(cx, cy+1, radius, color, -1.0);

	drawHalf2(cx, cy, radius, color);
}

} // namespace Graphics {

