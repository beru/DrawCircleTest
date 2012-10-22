
#include "graphics_impl_common.h"

namespace Graphics {

// http://www.softist.com/programming/drawcircle/drawcircle.htm
void drawCircle1 (int16_t cx, int16_t cy, uint16_t diameter, pixel_t color)
{
	uint16_t r = diameter / 2;
	#define SHIFT 7
	int xx = r << SHIFT;
	int yy = 0;
	int x = 0;
	int y = 0;
	while (yy < xx) {
		x = xx >> SHIFT;
		y = yy >> SHIFT;
		PutPixel(cx+x, cy+y, color);
		PutPixel(cx-x, cy-y, color);
		PutPixel(cx-x, cy+y, color);
		PutPixel(cx+x, cy-y, color);
		PutPixel(cx+y, cy+x, color);
		PutPixel(cx-y, cy-x, color);
		PutPixel(cx-y, cy+x, color);
		PutPixel(cx+y, cy-x, color);
		yy += xx >> SHIFT;
		xx -= yy >> SHIFT;
	}
}

// http://fussy.web.fc2.com/algo/algo2-1.htm
void drawCircle2 (int16_t cx, int16_t cy, uint16_t diameter, pixel_t color)
{
	uint16_t r = diameter / 2;
	int x = r;
	int y = 0;
	int F = -diameter + 3;
	while (x >= y) {
		PutPixel(cx - x, cy + y, color);
		PutPixel(cx + x, cy + y, color);
		PutPixel(cx - x, cy - y, color);
		PutPixel(cx + x, cy - y, color);
		PutPixel(cx - y, cy + x, color);
		PutPixel(cx + y, cy + x, color);
		PutPixel(cx - y, cy - x, color);
		PutPixel(cx + y, cy - x, color);
		if (F >= 0) {
			x--;
			F -= 4 * x;
		}
		y++;
		F += 4 * y + 2;
	}
}

// http://d.hatena.ne.jp/zariganitosh/20100318/1269006632
void drawCircle3 (int16_t cx, int16_t cy, uint16_t diameter, pixel_t color)
{
	uint16_t r = diameter / 2;
	int x = r;
	int y = 0;
	int f = -2 * r + 2;
	while (x > y) {
		PutPixel(cx - x, cy + y, color);
		PutPixel(cx + x, cy + y, color);
		PutPixel(cx - x, cy - y, color);
		PutPixel(cx + x, cy - y, color);
		PutPixel(cx - y, cy + x, color);
		PutPixel(cx + y, cy + x, color);
		PutPixel(cx - y, cy - x, color);
		PutPixel(cx + y, cy - x, color);
		if (f > -x) {
			f += -2 * x + 1;
			--x;
		}
		f += 2 * y + 1;
		++y;

	}
}

// http://willperone.net/Code/codecircle.php
// slightly faster algorithm posted by coyote
void CircleMidpoint (int xc, int yc, uint16_t diameter, pixel_t color)
{
	uint16_t r = diameter / 2;
	int x= 0;
	int y= r;
	int d= (2 - diameter) / 2 ;
	int dE= 3;
	int dSE= 5 - diameter ;
	int diff_dSE_dE= dSE - dE ;

	if (!r) return;
	PutPixel(xc-r, yc, color);
	PutPixel(xc+r, yc, color);
	PutPixel(xc, yc-r, color);
	PutPixel(xc, yc+r, color);

	while (y > x)    //only formulate 1/8 of circle
	{
		if( d >= 0 ) {
			d += diff_dSE_dE ;
			diff_dSE_dE += 2 ;
			--y ;
		}
		d += dE ;
		dE += 2 ;
		++x ;

		PutPixel(xc-x, yc-y, color);//upper left left
		PutPixel(xc+x, yc-y, color);//upper right right
		PutPixel(xc-y, yc-x, color);//upper upper left
		PutPixel(xc+y, yc-x, color);//upper upper right
		PutPixel(xc-y, yc+x, color);//lower lower left
		PutPixel(xc+y, yc+x, color);//lower lower right
		PutPixel(xc-x, yc+y, color);//lower left left
		PutPixel(xc+x, yc+y, color);//lower right right
	}
}

// http://willperone.net/Code/codecircle.php
// slightly faster algorithm posted by coyote
void FilledCircleMidpoint (int xc, int yc, uint16_t diameter, pixel_t color)
{
	uint16_t r = diameter / 2;
	int x= 0;
	int y= r;
	int d= (2 - diameter) / 2 ;
	int dE= 3;
	int dSE= 5 - diameter ;
	int diff_dSE_dE= dSE - dE ;

	if (!r) return;
	DrawHorizontalLine(xc-r, xc+r, yc, color);
	PutPixel(xc, yc-r, color);
	PutPixel(xc, yc+r, color);
	
	while (y > x)    //only formulate 1/8 of circle
	{
		if( d >= 0 ) {
			d += diff_dSE_dE ;
			diff_dSE_dE += 2 ;
			--y ;
		}
		d += dE ;
		dE += 2 ;
		++x ;
		
		DrawHorizontalLine(xc-x, xc+x, yc-y, color);
		DrawHorizontalLine(xc-y, xc+y, yc-x, color);
		DrawHorizontalLine(xc-y, xc+y, yc+x, color);
		DrawHorizontalLine(xc-x, xc+x, yc+y, color);
	}
}

// http://willperone.net/Code/codecircle.php
void CircleOptimized (int xc, int yc, uint16_t diameter, pixel_t color)
{
	uint16_t r = diameter / 2;
    unsigned int x= r, y= 0;//local coords     
    int          cd2= 0;    //current distance squared - radius squared

    if (!r) return; 
    PutPixel(xc-r, yc, color);
    PutPixel(xc+r, yc, color);
    PutPixel(xc, yc-r, color);
    PutPixel(xc, yc+r, color);
 
    while (x > y)    //only formulate 1/8 of circle
    {
        cd2-= (--x) - (++y);
        if (cd2 < 0) cd2+=x++;

        PutPixel(xc-x, yc-y, color);//upper left left
        PutPixel(xc-y, yc-x, color);//upper upper left
        PutPixel(xc+y, yc-x, color);//upper upper right
        PutPixel(xc+x, yc-y, color);//upper right right
        PutPixel(xc-x, yc+y, color);//lower left left
        PutPixel(xc-y, yc+x, color);//lower lower left
        PutPixel(xc+y, yc+x, color);//lower lower right
        PutPixel(xc+x, yc+y, color);//lower right right
     } 
}

// https://banu.com/blog/7/drawing-circles/
void BanuDrawCircle (int16_t cx, int16_t cy, uint16_t diameter, pixel_t color)
{
	uint16_t r = diameter / 2;
	int x, y;
	int l;
	int r2, y2;
	int y2_new;
	int ty;
	
	/* cos pi/4 = 185363 / 2^18 (approx) */
	l = (r * 185363) >> 18;
	
	/* At x=0, y=radius */
	y = r;
	
	r2 = y2 = y * y;
	ty = (2 * y) - 1;
	y2_new = r2 + 3;
	
	for (x=0; x<=l; ++x) {
		y2_new -= (2 * x) - 3;
		if ((y2 - y2_new) >= ty) {
			y2 -= ty;
			y -= 1;
			ty -= 2;
		}
		
		PutPixel(cx-y, cy-x, color);
		PutPixel(cx+y, cy-x, color);
		PutPixel(cx-y, cy+x, color);
		PutPixel(cx+y, cy+x, color);

		PutPixel(cx-x, cy-y, color);
		PutPixel(cx+x, cy-y, color);
		PutPixel(cx-x, cy+y, color);
		PutPixel(cx+x, cy+y, color);
		
	}
}

struct POINT
{
	long x;
	long y;
};

// http://dencha.ojaru.jp/programs_07/pg_graphic_09a2.html
void NewCircleAlgorithm (long diameter, POINT center, pixel_t col)
{
	long cy = diameter/2 + 1;
	long d = -diameter*diameter +4*cy*cy -4*cy +2;
	int dx = 4;
	int dy = -8*cy+8;
	POINT mirror_center = center;
	if ((diameter&1) ==0){
		mirror_center.x++;
		mirror_center.y++;
	}
	
	for (int cx=0; cx <= cy ; cx++) {

		if (d > 0) {
			d += dy;
			dy += 8;
			cy--;
		}
		d += dx;
		dx += 8;

		PutPixel (cy + center.x,  cx + center.y, col);        // 0-45     �x�̊�
		PutPixel (-cy + mirror_center.x,  cx + center.y, col); // 135-180  �x�̊�

		PutPixel (cx + center.x,  cy + center.y, col);        // 45-90    �x�̊�
		PutPixel (-cx + mirror_center.x,  cy + center.y, col); // 90-135   �x�̊�

		PutPixel (cy + center.x, -cx + mirror_center.y, col); // 315-360  �x�̊�
		PutPixel (-cy + mirror_center.x, -cx + mirror_center.y, col);  // 180-225  �x�̊�

		PutPixel (cx + center.x, -cy + mirror_center.y, col); // 270-315  �x�̊�
		PutPixel (-cx + mirror_center.x, -cy + mirror_center.y, col);  // 225-270  �x�̊�
	}
}

void DrawNewCircle (int16_t cx, int16_t cy, uint16_t diameter, pixel_t color)
{
	POINT p = {cx, cy};
	NewCircleAlgorithm (diameter, p, color);
}

// TODO: �`��͌�ł��B�i�̈ړ���񂾂��L�^���āA�`��͈�C�萬�ɂ��������������ǂ����낤�B

void OstCircle (int16_t cx, int16_t cy, uint16_t diameter, pixel_t color)
{
	int r = diameter / 2;
	int x = r;
	int y = 0;
	int d = 1 - r; // Pd has been initialised
	int dy = 1; // Pdy has been initialised
	int dxy = diameter - 1; // Pdxy has been initialised

	PutPixel (cx-r, cy, color);
	PutPixel (cx+r, cy, color);
	PutPixel (cy, cx-r, color);
	PutPixel (cy, cx+r, color);
	if (diameter <= 9) {
		while (dxy > 0) {
			y++;
			dy += 2;
			if (d < 0) { // select axial or diagonal move
				dxy -= 2; //increment like phase
				d += dy;
			}else {
				x--;
				dxy -= 4; //increment like phase
				d -= dxy;
			}
			PutPixel (cx-x, cx-y, color);
			PutPixel (cx+x, cx-y, color);
			PutPixel (cx-y, cx-x, color);
			PutPixel (cx+y, cx-x, color);
			PutPixel (cx-x, cx+y, color);
			PutPixel (cx+x, cx+y, color);
			PutPixel (cx-y, cx+x, color);
			PutPixel (cx+y, cx+x, color);

		}
	}else {
		// ���򂪏��Ȃ���
SA:
		y++;
		PutPixel (cx-x, cx-y, color);
		PutPixel (cx+x, cx-y, color);
		PutPixel (cx-y, cx-x, color);
		PutPixel (cx+y, cx-x, color);
		PutPixel (cx-x, cx+y, color);
		PutPixel (cx+x, cx+y, color);
		PutPixel (cx-y, cx+x, color);
		PutPixel (cx+y, cx+x, color);

		dxy -= 2;
		dy += 2;
		d += dy;
		if (d < 0) {
			goto SA;
		}
SD:
		x--;
		y++;
		PutPixel (cx-x, cx-y, color);
		PutPixel (cx+x, cx-y, color);
		PutPixel (cx-y, cx-x, color);
		PutPixel (cx+y, cx-x, color);
		PutPixel (cx-x, cx+y, color);
		PutPixel (cx+x, cx+y, color);
		PutPixel (cx-y, cx+x, color);
		PutPixel (cx+y, cx+x, color);
		
		dxy -= 4;
		if (dxy <= 0) {
			return;
		}
		dy += 2;
		d -= dxy;
		if (d < 0) {
			goto SA;
		}else {
			goto SD;
		}
	}
}

void DrawCircle (int16_t cx, int16_t cy, uint16_t diameter, pixel_t color)
{
//	drawCircle1
//	drawCircle2
//	drawCircle3
//	CircleMidpoint
//	CircleOptimized
//	BanuDrawCircle
	DrawNewCircle
		(cx, cy, diameter, color);
}

void DrawFilledCircle(int16_t cx, int16_t cy, uint16_t diameter, pixel_t color)
{
	FilledCircleMidpoint (cx, cy, diameter, color);
}

} // namespace Graphics


