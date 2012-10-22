
#include "graphics_impl_common.h"

namespace Graphics {

void DrawRectangle(int16_t x, int16_t y, uint16_t w, uint16_t h, uint32_t color, const BorderStyle& border)
{
	// îwåiï`âÊ
	FillRect(x, y, w, h, color);
	
	// éüÇ…ògê¸Çï`âÊ
	if (border.width == 0) {
		return;
	}
	// top
	FillRect(x, y, w, border.width, border.color);
	// left
	FillRect(x, y+border.width, border.width, h-border.width*2, border.color);
	// right
	FillRect(x+w-border.width, y+border.width, border.width, h-border.width*2, border.color);
	// bottom
	FillRect(x, y+h-border.width, w, border.width, border.color);
}

} // namespace Graphics
