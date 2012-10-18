#include "graphics_impl_common.h"
#include <math.h>
#include <algorithm>

#include <vector>

namespace {

float lengths[1024];
float areas[1024];
double sum = 0;
double deg45Len = sqrt(2.0) / 2;

// 45“x‚Ü‚Å‚ğ“¯‚¶‚‚³‚Å“™•ª‚µ‚½Û‚ÌA’·‚³‚Æ–ÊÏ‚ğZo‚µ‚Ä•\‚É“ü‚ê‚éB
void calcTable()
{
	for (int i=1; i<=countof(lengths); ++i) {
		double y = (deg45Len * i) / countof(lengths);
		double x = 1 - sqrt(1 - y*y);
		sum += x;
		lengths[i-1] = x;
		areas[i-1] = sum;
	}
}

} // namespace

namespace Graphics {

void DrawFilledCircleAA(float cx, float cy, float diameter, pixel_t color)
{
	float radius = diameter / 2.0;
	float radius2 = radius * radius;
	float ty = cy - radius;
	float by = cy + radius;
	float lx = cx - radius;
	float rx = cx + radius;
	
	int ity = floor(ty);
	int iby = ceil(by);
	int ilx = floor(lx);
	int irx = ceil(rx);
	
	// ‰~‚Ì’†‰›‚Ì‰¡ü‚Ìã‘¤‚Ì‰æ‘f‹«ŠE‚Æ‚ÌÚ“_‚ğ‹‚ß‚é
	float y0 = floor(cy);
	float dy = cy - y0;
	float dy2 = dy * dy;
	float dx = sqrt(radius2 - dy2);	// ’†‰›‚Æ‚Ì‹——£
	float lx0 = cx - dx;			// â‘ÎÀ•W‚É•ÏŠ·
	float rx0 = cx + dx;			// ‰~‚Ì‘ÎÛ«
	// ‰~‚Ì’†‰›‚Ì‰¡ü‚Ì‰º‘¤‚Ì‰æ‘f‹«ŠE‚Æ‚ÌÚ“_‚ğ‹‚ß‚é
	float y1 = y0 + 1.0;
	dy += 1.0;
	dy2 = dy * dy;
	dx = sqrt(radius2 - dy2);
	lx = cx - dx;
	rx = cx + dx;
	ilx = floor(lx);
	irx = floor(rx);
	while (y1 <= iby) {
		// 1‰æ‘f‰º‚É‚¸‚ç‚µ‚ÄA‰¡ü‚Ì‰º‘¤‚Ì‰æ‘f‹«ŠE‚Æ‚ÌÚ“_‚ğ‹‚ß‚é
		float olx = lx;
		float orx = rx;
		float oilx = ilx;
		float oirx = irx;
		y1 += 1.0;
		dy += 1.0;
		dy2 = dy * dy;
		dx = sqrt(radius2 - dy2);
		lx = cx - dx;
		rx = cx + dx;
		ilx = floor(lx);
		irx = floor(rx);
		
		dx = lx - olx;
		if (ilx - oilx) {
			// ‰¡2‰æ‘f‚É“n‚Á‚Ä‘S•”“h‚è‚Â‚Ô‚³‚ê‚Ä‚¢‚È‚¢—Ìˆæ‚ª‚ ‚é
			// OŠpŒ`‚Ì–ÊÏ‚Ì”ä—¦A
			// ’·‚³‚Ì”ä‚Å‚‚³‚ªo‚¹‚é
			float dx0 = ilx - olx;			// ¶‘¤‚Ì‰æ‘f’†‚É‚Í‚İo‚Ä‚¢‚éOŠpŒ`‚Ì‰¡•
			float ratio = dx0 / dx;			// ¶‘¤‚Ì‰æ‘f’†‚É‚Í‚İo‚Ä‚¢‚éOŠpŒ`‚Ì”ä—¦
			float la = (dx0 * ratio) / 2.0;	// ¶‘¤‚Ì‰æ‘f’†‚ÌOŠpŒ`‚Ì–ÊÏ
			float ra = (dx / 2) - la + (1.0 - (lx - ilx));	// ‰E‘¤‚Ì‰æ‘f’†‚ÌOŠpŒ`‚Ì–ÊÏ
			putPixel(oilx, y1, la*255);
			putPixel(oilx+1, y1, ra*255);
			DrawHorizontalLine(oilx+2, irx, y1, 255);
		}else {
			// Ø‚èÌ‚Ä‚Ä“¯‚¶‰æ‘f‚É‚ ‚éê‡‚Í‰¡2‰æ‘f‚É‚Ü‚½‚ª‚Á‚Ä‚¢‚È‚¢B
			float a = (ilx + 1.0 - lx) + dx / 2.0;
			putPixel(oilx, y1, a*255);
			DrawHorizontalLine(oilx+1, irx, y1, 255);
		}
		
		dx = orx - rx;
		if (oirx - irx) {
			float dx0 = orx - oirx;			// ‰E‘¤‚Ì‰æ‘f’†‚É‚Í‚İo‚Ä‚¢‚éOŠpŒ`‚Ì‰¡•
			float ratio = dx0 / dx;			// ‰E‘¤‚Ì‰æ‘f’†‚É‚Í‚İo‚Ä‚¢‚éOŠpŒ`‚Ì”ä—¦
			float ra = (dx0 * ratio) / 2.0;	// ‰E‘¤‚Ì‰æ‘f’†‚ÌOŠpŒ`‚Ì–ÊÏ
			float la = (dx / 2) - ra + (rx - irx);	// ¶‘¤‚Ì‰æ‘f’†‚ÌOŠpŒ`‚Ì–ÊÏ
			putPixel(irx, y1, la*255);
			putPixel(irx+1, y1, ra*255);
		}else {
			float a = (rx - irx) + dx / 2.0;
			putPixel(irx, y1, a*255);
		}
		
	}
	
}


} // namespace Graphics

