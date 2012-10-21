#ifndef GRAPHICS_H_INCLUDED__
#define GRAPHICS_H_INCLUDED__

namespace Graphics {

typedef uint32_t pixel_t;

void SetCanvas(void* buff, uint16_t width, uint16_t height, int32_t stride);

void SetClippingRect(int16_t x, int16_t y, uint16_t w, uint16_t h);

static inline
pixel_t MakePixel(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	return (a<<24) | (b<<16) | (g<<8) | r;
}

void PutPixel(uint16_t x, uint16_t y, pixel_t color);
void FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, pixel_t color);

void DrawHorizontalLine(int16_t x1, int16_t x2, int16_t y2, pixel_t color);
void DrawVerticalLine(int16_t x, int16_t y1, int16_t y2, pixel_t color);
void DrawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, pixel_t color);

struct BorderStyle
{
	uint32_t color;
	uint8_t width;
	uint8_t radius;
};

void DrawRectangle(int16_t x, int16_t y, uint16_t w, uint16_t h, pixel_t color, const BorderStyle& border);

void DrawCircle(int16_t cx, int16_t cy, uint16_t diameter, pixel_t color);
void DrawFilledCircle(int16_t cx, int16_t cy, uint16_t diameter, pixel_t color);
void DrawFilledCircleAA(float cx, float cy, float diameter, pixel_t color);
void DrawFilledCircleAA2(float cx, float cy, float diameter, pixel_t color);

struct Vertex
{
	float x;
	float y;
};

void DrawTriangle(const Vertex& v1, const Vertex& v2, const Vertex& v3);

} // namespace Graphics

#endif // ifndef GRAPHICS_H_INCLUDED__
