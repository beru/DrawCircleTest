

#include "main_plus.h"
#include <windowsx.h>
#include <assert.h>

#include "stdint.h"
#include <vector>

#include "winutil.h"
#include "arrayutil.h"

#include "graphics/graphics.h"
#include <tchar.h>
#include <stdio.h>

#include "timer.h"

#define GDIPVER 0x0110
#include <gdiplus.h>

namespace {

HWND hWnd;
HBITMAP hBMP;
std::vector<uint8_t> bmiBuff(sizeof(BITMAPINFO) + sizeof(RGBQUAD) * 256);
BITMAPINFO* pBMI;
void* pBits;
HDC hMemDC;
HFONT hFont;

float x_ = 500;
float y_ = 500.5;
float radius_;
float prevRadius_;
float prevX_ = x_;
float prevY_ = y_;

} // anonymous namespace

void OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	::SetTimer(hWnd, 100, 10, 0);
	
	pBMI = (BITMAPINFO*) &bmiBuff[0];
	BITMAPINFO& bmi = *pBMI;
	
	int width = GetSystemMetrics(SM_CXFULLSCREEN);
	int height = GetSystemMetrics(SM_CYFULLSCREEN);
	int bitsPerPixel = 32;
	width = (width + 3) & (~3);
	
	radius_ = height / 3;
	prevRadius_ = radius_;

#if 0
	RGBQUAD rgb;
	for (size_t i=0; i<256; ++i) {
		rgb.rgbBlue = rgb.rgbGreen = rgb.rgbRed = i;
		bmi.bmiColors[i] = rgb;
	}
	hBMP = CreateDIB(width, -height, 8, bmi, pBits);
#else
	hBMP = CreateDIB(width, -height, bitsPerPixel, bmi, pBits);
#endif
	
	HDC hWndDC = ::GetDC(hWnd);
	hMemDC = ::CreateCompatibleDC(hWndDC);
	::SetMapMode(hMemDC, ::GetMapMode(hWndDC));
	::ReleaseDC(hWnd, hWndDC);
	::SelectObject(hMemDC, hBMP);

	Graphics::SetCanvas(pBits, width, height, width*4);

	LOGFONT lf = {0};
	lf.lfHeight = -MulDiv(12, GetDeviceCaps(hMemDC, LOGPIXELSY), 72);
	lf.lfQuality = CLEARTYPE_QUALITY;
	hFont = ::CreateFontIndirect(&lf);
	::SelectObject(hMemDC, hFont);
}

void OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	::DeleteDC(hMemDC);
	::DeleteObject(hBMP);
	::DeleteObject(hFont);
}

void OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	hdc = BeginPaint(hWnd, &ps);
	
	const RECT& rec = ps.rcPaint;
	::BitBlt(
		hdc,
		rec.left,
		rec.top,
		rec.right - rec.left,
		rec.bottom - rec.top,
		hMemDC,
		rec.left,
		rec.top,
		SRCCOPY
	);
	
	EndPaint(hWnd, &ps);
}

void OnMouseDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
//	if (wParam & MK_LBUTTON) {
//	}
}

void OnMouseUp(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
//	if (wParam & MK_LBUTTON) {
//	}
}

void OnMouseMove(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
//	if (wParam & MK_LBUTTON) {
//	}
	//prevX_ = x_;
	//prevY_ = y_;

	//x_ = GET_X_LPARAM(lParam);
	//y_ = GET_Y_LPARAM(lParam);
}

void OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (!IsWindow(hWnd)) {
		return;
	}
	
	RECT rec;
	rec.left = prevX_-prevRadius_-5;
	rec.top = prevY_-prevRadius_-5;
	rec.right = prevX_+prevRadius_+5;
	rec.bottom = prevY_+prevRadius_+5;

	if (0) {
		RECT rect;
		rect.left = 0;
		rect.top = 0;
		rect.right = pBMI->bmiHeader.biWidth;
		rect.bottom = abs(pBMI->bmiHeader.biHeight);
		FillRect(hMemDC, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
	}
	Graphics::FillAll(0xFF303030);
//	Graphics::FillRect(0,0,2560,1440, 0xFF303030);
	
	Timer timer;
	
	float diameter = radius_*2;

#if 1
//	Graphics::FillRect(x_-radius_, y_-radius_, diameter, diameter, 0x7F0000FF);
//	Graphics::FillRect(x_-radius_+50, y_-radius_+50, diameter, diameter, 0x7F00FF00);

#if 0
	Graphics::DrawFilledCircleAA(x_, y_, diameter, 0x7F0000FF);
	Graphics::DrawFilledCircleAA(x_+50, y_+50, diameter, 0x7F00FF00);
#elif 0
	Graphics::DrawFilledEllipse(x_+50, y_+50, 250, 500, 0x7F0000FF);
	Graphics::DrawFilledEllipse(x_+150, y_+150, 150, 600, 0x7F00FF00);
#else
	Graphics::DrawFilledEllipse(x_+50, y_+50, 800, 100, 0x7F00FF00);
	Graphics::DrawFilledEllipse(x_ + 450, y_ + 250, 100, 800, 0x7F00FF00);
	Graphics::DrawFilledEllipse(x_ + 850, y_ + 450, 800, 200, 0x7F00FF00);
#endif

#else
	{
		using namespace Gdiplus;
		Gdiplus::Graphics g(hMemDC);
		g.SetSmoothingMode(SmoothingModeAntiAlias8x8);
		Gdiplus::SolidBrush b(Color(0x7F0000FF));
		Gdiplus::SolidBrush b2(Color(0x7F00FF00));
		g.FillEllipse(&b, x_-radius_, y_-radius_, diameter, diameter);
		g.FillEllipse(&b2, x_-radius_+50, y_-radius_+50, diameter, diameter);
	}
#endif
	
	double elapsed = timer.ElapsedSecond() * 1000;
	
	rec.left = 0;
	rec.top = 0;
	rec.right = pBMI->bmiHeader.biWidth;
	rec.bottom = abs(pBMI->bmiHeader.biHeight);
	::InvalidateRect(hWnd, &rec, TRUE);
	
	TCHAR str[32];
	_stprintf(str, _T("%f"), elapsed);
	::SetTextColor(hMemDC, -1);
	::TextOut(hMemDC, 10, 10, str, _tcslen(str));
	rec.left = 10;
	rec.top = 10;
	rec.right = 100;
	rec.bottom = 100;
	::InvalidateRect(hWnd, &rec, TRUE);
	
	prevX_ = x_;
	prevY_ = y_;
	prevRadius_ = radius_;
	
	x_ += 0.1;
	y_ -= 0.1;
//	radius_ += 0.1;
}

