#include "graphics_impl_common.h"

namespace Graphics {

/*
	
	http://devmaster.net/forums/topic/1145-advanced-rasterization/
	
*/

static inline
void DrawTriangle1(const Vertex& v1, const Vertex& v2, const Vertex& v3)
{
	float y1 = v1.y;
	float y2 = v2.y;
	float y3 = v3.y;

	float x1 = v1.x;
	float x2 = v2.x;
	float x3 = v3.x;

	// Bounding rectangle
	int minx = (int)min(x1, x2, x3);
	int maxx = (int)max(x1, x2, x3);
	int miny = (int)min(y1, y2, y3);
	int maxy = (int)max(y1, y2, y3);
	
	pixel_t* colorBuffer = getPixelPtr(0, miny);
	
	// Scan through bounding rectangle
	for (int y=miny; y<maxy; y++) {
		for (int x=minx; x<maxx; x++) {
			// When all half-space functions positive, pixel is in triangle
			if (
				(x1 - x2) * (y - y1) - (y1 - y2) * (x - x1) > 0 &&
				(x2 - x3) * (y - y2) - (y2 - y3) * (x - x2) > 0 &&
				(x3 - x1) * (y - y3) - (y3 - y1) * (x - x3) > 0
			)
			{
				colorBuffer[x] = 0x00FFFFFF; // White
			}
		}
		OffsetPtr(colorBuffer, getLineOffset());
	}
}

static inline
void DrawTriangle2(const Vertex& v1, const Vertex& v2, const Vertex& v3)
{
	float y1 = v1.y;
	float y2 = v2.y;
	float y3 = v3.y;

	float x1 = v1.x;
	float x2 = v2.x;
	float x3 = v3.x;

	// Deltas
	float Dx12 = x1 - x2;
	float Dx23 = x2 - x3;
	float Dx31 = x3 - x1;

	float Dy12 = y1 - y2;
	float Dy23 = y2 - y3;
	float Dy31 = y3 - y1;

	// Bounding rectangle
	int minx = (int)min(x1, x2, x3);
	int maxx = (int)max(x1, x2, x3);
	int miny = (int)min(y1, y2, y3);
	int maxy = (int)max(y1, y2, y3);

	pixel_t* colorBuffer = getPixelPtr(0, miny);

	// Constant part of half-edge functions
	float C1 = Dy12 * x1 - Dx12 * y1;
	float C2 = Dy23 * x2 - Dx23 * y2;
	float C3 = Dy31 * x3 - Dx31 * y3;

	float Cy1 = C1 + Dx12 * miny - Dy12 * minx;
	float Cy2 = C2 + Dx23 * miny - Dy23 * minx;
	float Cy3 = C3 + Dx31 * miny - Dy31 * minx;

	// Scan through bounding rectangle
	for (int y = miny; y < maxy; y++) {
		// Start value for horizontal scan
		float Cx1 = Cy1;
		float Cx2 = Cy2;
		float Cx3 = Cy3;
		for (int x = minx; x < maxx; x++) {
			if (Cx1 > 0 && Cx2 > 0 && Cx3 > 0) {
				colorBuffer[x] = 0x00FFFFFF;	// White
			}
			Cx1 -= Dy12;
			Cx2 -= Dy23;
			Cx3 -= Dy31;
		}
		Cy1 += Dx12;
		Cy2 += Dx23;
		Cy3 += Dx31;
		OffsetPtr(colorBuffer, getLineOffset());
	}
	
}

inline int iround(float x)
{
  int t;

  __asm
  {
    fld  x
    fistp t
  }

  return t;
}

void DrawTriangle3(const Vertex& v1, const Vertex& v2, const Vertex& v3)
{
	// 28.4 fixed-point coordinates
	const int Y1 = iround(16.0f * v1.y);
	const int Y2 = iround(16.0f * v2.y);
	const int Y3 = iround(16.0f * v3.y);

	const int X1 = iround(16.0f * v1.x);
	const int X2 = iround(16.0f * v2.x);
	const int X3 = iround(16.0f * v3.x);

	// Deltas
	const int DX12 = X1 - X2;
	const int DX23 = X2 - X3;
	const int DX31 = X3 - X1;

	const int DY12 = Y1 - Y2;
	const int DY23 = Y2 - Y3;
	const int DY31 = Y3 - Y1;

	// Fixed-point deltas
	const int FDX12 = DX12 << 4;
	const int FDX23 = DX23 << 4;
	const int FDX31 = DX31 << 4;

	const int FDY12 = DY12 << 4;
	const int FDY23 = DY23 << 4;
	const int FDY31 = DY31 << 4;

	// Bounding rectangle
	int minx = (min(X1, X2, X3) + 0xF) >> 4;
	int maxx = (max(X1, X2, X3) + 0xF) >> 4;
	int miny = (min(Y1, Y2, Y3) + 0xF) >> 4;
	int maxy = (max(Y1, Y2, Y3) + 0xF) >> 4;

	pixel_t* colorBuffer = getPixelPtr(0, miny);

	// Half-edge constants
	int C1 = DY12 * X1 - DX12 * Y1;
	int C2 = DY23 * X2 - DX23 * Y2;
	int C3 = DY31 * X3 - DX31 * Y3;

	// Correct for fill convention
	if (DY12 < 0 || (DY12 == 0 && DX12 > 0)) C1++;
	if (DY23 < 0 || (DY23 == 0 && DX23 > 0)) C2++;
	if (DY31 < 0 || (DY31 == 0 && DX31 > 0)) C3++;

	int CY1 = C1 + DX12 * (miny << 4) - DY12 * (minx << 4);
	int CY2 = C2 + DX23 * (miny << 4) - DY23 * (minx << 4);
	int CY3 = C3 + DX31 * (miny << 4) - DY31 * (minx << 4);

	for (int y = miny; y < maxy; y++) {
		int CX1 = CY1;
		int CX2 = CY2;
		int CX3 = CY3;
		for (int x = minx; x < maxx; x++) {
			if (CX1 > 0 && CX2 > 0 && CX3 > 0) {
				colorBuffer[x] = 0x00FFFFFF;
			}
			CX1 -= FDY12;
			CX2 -= FDY23;
			CX3 -= FDY31;
		}
		CY1 += FDX12;
		CY2 += FDX23;
		CY3 += FDX31;
		OffsetPtr(colorBuffer, getLineOffset());
	}
	
}

void DrawTriangle4(const Vertex& v1, const Vertex& v2, const Vertex& v3)
{
	// 28.4 fixed-point coordinates
	const int Y1 = iround(16.0f * v1.y);
	const int Y2 = iround(16.0f * v2.y);
	const int Y3 = iround(16.0f * v3.y);
	const int X1 = iround(16.0f * v1.x);
	const int X2 = iround(16.0f * v2.x);
	const int X3 = iround(16.0f * v3.x);
	// Deltas
	const int DX12 = X1 - X2;
	const int DX23 = X2 - X3;
	const int DX31 = X3 - X1;
	const int DY12 = Y1 - Y2;
	const int DY23 = Y2 - Y3;
	const int DY31 = Y3 - Y1;
	// Fixed-point deltas
	const int FDX12 = DX12 << 4;
	const int FDX23 = DX23 << 4;
	const int FDX31 = DX31 << 4;
	const int FDY12 = DY12 << 4;
	const int FDY23 = DY23 << 4;
	const int FDY31 = DY31 << 4;
	// Bounding rectangle
	int minx = (min(X1, X2, X3) + 0xF) >> 4;
	int maxx = (max(X1, X2, X3) + 0xF) >> 4;
	int miny = (min(Y1, Y2, Y3) + 0xF) >> 4;
	int maxy = (max(Y1, Y2, Y3) + 0xF) >> 4;
	// Block size, standard 8x8 (must be power of two)
	const int q = 8;
	// Start in corner of 8x8 block
	minx &= ~(q - 1);
	miny &= ~(q - 1);
	pixel_t* colorBuffer = getPixelPtr(0, miny);
	// Half-edge constants
	int C1 = DY12 * X1 - DX12 * Y1;
	int C2 = DY23 * X2 - DX23 * Y2;
	int C3 = DY31 * X3 - DX31 * Y3;
	// Correct for fill convention
	if (DY12 < 0 || (DY12 == 0 && DX12 > 0)) C1++;
	if (DY23 < 0 || (DY23 == 0 && DX23 > 0)) C2++;
	if (DY31 < 0 || (DY31 == 0 && DX31 > 0)) C3++;
	// Loop through blocks
	for (int y = miny; y < maxy; y += q) {
		for (int x = minx; x < maxx; x += q) {
			// Corners of block
			int x0 = x << 4;
			int x1 = (x + q - 1) << 4;
			int y0 = y << 4;
			int y1 = (y + q - 1) << 4;
			// Evaluate half-space functions
			bool a00 = C1 + DX12 * y0 - DY12 * x0 > 0;
			bool a10 = C1 + DX12 * y0 - DY12 * x1 > 0;
			bool a01 = C1 + DX12 * y1 - DY12 * x0 > 0;
			bool a11 = C1 + DX12 * y1 - DY12 * x1 > 0;
			int a = (a00 << 0) | (a10 << 1) | (a01 << 2) | (a11 << 3);
			bool b00 = C2 + DX23 * y0 - DY23 * x0 > 0;
			bool b10 = C2 + DX23 * y0 - DY23 * x1 > 0;
			bool b01 = C2 + DX23 * y1 - DY23 * x0 > 0;
			bool b11 = C2 + DX23 * y1 - DY23 * x1 > 0;
			int b = (b00 << 0) | (b10 << 1) | (b01 << 2) | (b11 << 3);
			bool c00 = C3 + DX31 * y0 - DY31 * x0 > 0;
			bool c10 = C3 + DX31 * y0 - DY31 * x1 > 0;
			bool c01 = C3 + DX31 * y1 - DY31 * x0 > 0;
			bool c11 = C3 + DX31 * y1 - DY31 * x1 > 0;
			int c = (c00 << 0) | (c10 << 1) | (c01 << 2) | (c11 << 3);
			// Skip block when outside an edge
			if (a == 0x0 || b == 0x0 || c == 0x0) continue;
			pixel_t* buffer = colorBuffer;
			// Accept whole block when totally covered
			if (a == 0xF && b == 0xF && c == 0xF) {
				for (int iy = 0; iy < q; iy++) {
					for (int ix = x; ix < x + q; ix++) {
						buffer[ix] = 0x00007F00; // Green
					}
					OffsetPtr(buffer, getLineOffset());
				}
			}else {// Partially covered block
				int CY1 = C1 + DX12 * y0 - DY12 * x0;
				int CY2 = C2 + DX23 * y0 - DY23 * x0;
				int CY3 = C3 + DX31 * y0 - DY31 * x0;
				for (int iy = y; iy < y + q; iy++) {
					int CX1 = CY1;
					int CX2 = CY2;
					int CX3 = CY3;
					for (int ix = x; ix < x + q; ix++) {
						if (CX1 > 0 && CX2 > 0 && CX3 > 0) {
							buffer[ix] = 0x0000007F; // Blue
						}
						CX1 -= FDY12;
						CX2 -= FDY23;
						CX3 -= FDY31;
					}
					CY1 += FDX12;
					CY2 += FDX23;
					CY3 += FDX31;
					OffsetPtr(buffer, getLineOffset());
				}
			}
		}
		OffsetPtr(colorBuffer, q*getLineOffset());
	}
}

void DrawTriangle5(const Vertex& v1, const Vertex& v2, const Vertex& v3)
{
	// 28.4 fixed-point coordinates
	const int Y1 = iround(16.0f * v1.y);
	const int Y2 = iround(16.0f * v2.y);
	const int Y3 = iround(16.0f * v3.y);

	const int X1 = iround(16.0f * v1.x);
	const int X2 = iround(16.0f * v2.x);
	const int X3 = iround(16.0f * v3.x);
	// Deltas
/*
  This was really easy. I fit 6 words into XMMs and could do all the
  subtraction and shifting.
*/
	const int DX12 = X1 - X2;
	const int DX23 = X2 - X3;
	const int DX31 = X3 - X1;

	const int DY12 = Y1 - Y2;
	const int DY23 = Y2 - Y3;
	const int DY31 = Y3 - Y1;

	// Fixed-point deltas
	const int FDX12 = DX12 << 4;
	const int FDX23 = DX23 << 4;
	const int FDX31 = DX31 << 4;
	const int FDY12 = DY12 << 4;
	const int FDY23 = DY23 << 4;
	const int FDY31 = DY31 << 4;

	// Bounding rectangle
/*
  These four went in 2 clock cycles :)
*/
	int minx = (min(X1, X2, X3) + 0xF) >> 4;
	int maxx = (max(X1, X2, X3) + 0xF) >> 4;
	int miny = (min(Y1, Y2, Y3) + 0xF) >> 4;
	int maxy = (max(Y1, Y2, Y3) + 0xF) >> 4;

	// Block size, standard 8x8 (must be power of two)
	const int q = 8;
/*
  I found out this to be very interesting dynamically. There were situations
  where 4 or 16 was better. Imagine triangles from 4x4px bounding in size
  upto 500x500 in size...
*/
	// Start in corner of 8x8 block
	minx &= ~(q - 1); // This and later is all in 64-bit x86 ASM without SSE
	miny &= ~(q - 1);

	pixel_t* colorBuffer = getPixelPtr(0, miny);
	// Half-edge constants
	int C1 = DY12 * X1 - DX12 * Y1; // SSE only has instructions for WORDs
	int C2 = DY23 * X2 - DX23 * Y2; // but this needs at least DWORDs and
	int C3 = DY31 * X3 - DX31 * Y3; // doing 6 multiplys with IMUL is faster
	// than converting it to SSE and doing 2*3 MULUDQs with additional sign-
	// checking because UDQ is UNsigned :(

	// Correct for fill convention
	if(DY12 < 0 || (DY12 == 0 && DX12 > 0)) C1++;
	if(DY23 < 0 || (DY23 == 0 && DX23 > 0)) C2++;
	if(DY31 < 0 || (DY31 == 0 && DX31 > 0)) C3++;
	// Loop through blocks
	for(int y = miny; y < maxy; y += q)
	{
		for(int x = minx; x < maxx; x += q)
		{
			// Corners of block
			int x0 = x << 4;
			int x1 = (x + q - 1) << 4;
			int y0 = y << 4;
			int y1 = (y + q - 1) << 4;

			// Evaluate half-space functions
/*
  This one here is very painful and even in case of a right triangle sitting in
  one half of the bounding box it takes 24 IMULs for every 8x8 pixel region
  empty or not. One or two optimizations are possible, though. You cast these
  bools to ints here, but actually you could just add these results without
  shifting (a00+a10+a01+a11) because you only check whether its all 0 or
  all 1. Then it becomes 0 or 4. Only loss is that you don't know which ones
  where 1 and which ones 0 (may be necessary with masking). The definite
  one optimization is that you don't have to wait until c to check for 0, but
  you can to it after every four bools. Bail out of the loop quickly after a==0
  so you will cut off these painful 16 IMULs, 16 ADDs/SUBs, 8 CMPs, 4 SHLs
  and 4 ORs. That on average will improve throughput by 10-35% depending
  on the triangles.
*/
			bool a00 = C1 + DX12 * y0 - DY12 * x0 > 0;
			bool a10 = C1 + DX12 * y0 - DY12 * x1 > 0;
			bool a01 = C1 + DX12 * y1 - DY12 * x0 > 0;
			bool a11 = C1 + DX12 * y1 - DY12 * x1 > 0;
			int a = (a00 << 0) | (a10 << 1) | (a01 << 2) | (a11 << 3);
	
			bool b00 = C2 + DX23 * y0 - DY23 * x0 > 0;
			bool b10 = C2 + DX23 * y0 - DY23 * x1 > 0;
			bool b01 = C2 + DX23 * y1 - DY23 * x0 > 0;
			bool b11 = C2 + DX23 * y1 - DY23 * x1 > 0;
			int b = (b00 << 0) | (b10 << 1) | (b01 << 2) | (b11 << 3);
	
			bool c00 = C3 + DX31 * y0 - DY31 * x0 > 0;
			bool c10 = C3 + DX31 * y0 - DY31 * x1 > 0;
			bool c01 = C3 + DX31 * y1 - DY31 * x0 > 0;
			bool c11 = C3 + DX31 * y1 - DY31 * x1 > 0;
			int c = (c00 << 0) | (c10 << 1) | (c01 << 2) | (c11 << 3);

			// Skip block when outside an edge
/*
  This one can be cleverly done with
  #define has_nullbyte_(x) ((x - 0x01010101) & ~x & 0x80808080)
  hint: search for "null" at
  http://www.azillionmonkeys.com/qed/asmexample.html
  but I didn't need it at all because of the optimizations above
*/
			if(a == 0x0 || b == 0x0 || c == 0x0) continue;
			pixel_t* buffer = colorBuffer;

			// Accept whole block when totally covered
/*
  By fusing a,b,c together earlier I could make here only one test which is
  if(c==0xFFF) - that is when c=a<<8|b<<4|c
*/
			if(a == 0xF && b == 0xF && c == 0xF)
			{
				for(int iy = 0; iy < q; iy++)
				{
					for(int ix = x; ix < x + q; ix++)
					{
						buffer[ix] = 0x00007F00; // Green
/*
  This part here gave a really good boost. I switched to SSE for a second
  here and shuffled the color to all parts of the 128-bit register. It could
  write 16 bytes per clock and with some macro magic, I made it calculate
  from the q how much of these it needed. The inner loop is too tight to use
  it at all so I left it out :)
*/
					}
					OffsetPtr(buffer, getLineOffset());
				}
			}
			else // Partially covered block
			{
				int CY1 = C1 + DX12 * y0 - DY12 * x0;
				int CY2 = C2 + DX23 * y0 - DY23 * x0;
				int CY3 = C3 + DX31 * y0 - DY31 * x0;

				for(int iy = y; iy < y + q; iy++)
				{
					int CX1 = CY1;
					int CX2 = CY2;
					int CX3 = CY3;

					for(int ix = x; ix < x + q; ix++)
					{
						if(CX1 > 0 && CX2 > 0 && CX3 > 0)
						{
							buffer[ix] = 0x0000007F; // Blue
						}

						CX1 -= FDY12;
						CX2 -= FDY23;
						CX3 -= FDY31;
					}

					CY1 += FDX12;
					CY2 += FDX23;
					CY3 += FDX31;
					OffsetPtr(buffer, getLineOffset());
				}
			}
		}
		OffsetPtr(colorBuffer, q * getLineOffset());
	}

}

void DrawTriangle(const Vertex& v1, const Vertex& v2, const Vertex& v3)
{
	DrawTriangle4(v1,v2,v3);
}


} // namespace Graphics