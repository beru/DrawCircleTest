

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

namespace {

HWND hWnd;
HBITMAP hBMP;
std::vector<uint8_t> bmiBuff(sizeof(BITMAPINFO) + sizeof(RGBQUAD) * 256);
//BITMAPINFO bmi;
void* pBits;
HDC hMemDC;

} // anonymous namespace

void OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	BITMAPINFO* pBMI = (BITMAPINFO*) &bmiBuff[0];
	BITMAPINFO& bmi = *pBMI;
	
	int width = 1920;
	int height = 1080;
	int bitsPerPixel = 32;
	
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
}

void OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	::DeleteDC(hMemDC);
	::DeleteObject(hBMP);
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

static float x_ = 201.5;
static float y_ = 810.0;
static float radius_ = 200;
static float prevX_ = x_;
static float prevY_ = y_;
static float prevRadius_ = radius_;

void OnMouseMove(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
//	if (wParam & MK_LBUTTON) {
//	}
	//prevX_ = x_;
	//prevY_ = y_;

	//x_ = GET_X_LPARAM(lParam);
	//y_ = GET_Y_LPARAM(lParam);
}

void OnTime(HWND hWnd)
{
	if (!IsWindow(hWnd)) {
		return;
	}
	
	RECT rec;
	rec.left = prevX_-prevRadius_-5;
	rec.top = prevY_-prevRadius_-5;
	rec.right = prevX_+prevRadius_+5;
	rec.bottom = prevY_+prevRadius_+5;
	Graphics::FillRect(rec.left, rec.top, rec.right-rec.left, rec.bottom-rec.top, 0);

	Timer timer;
	float diameter = radius_*2;
	Graphics::DrawFilledCircleAA(x_, y_, diameter, 0x00FF00);
	
	double elapsed = timer.ElapsedSecond() * 1000;

	rec.left = x_-radius_-5;
	rec.top = y_-radius_-5;
	rec.right = x_ + radius_+5;
	rec.bottom = y_ + radius_+5;
	::InvalidateRect(hWnd, &rec, FALSE);

	TCHAR str[32];
	_stprintf(str, _T("%f"), elapsed);
	::SetTextColor(hMemDC, -1);
	::TextOut(hMemDC, 10, 10, str, _tcslen(str));
	rec.left = 10;
	rec.top = 10;
	rec.right = 100;
	rec.bottom = 100;
	::InvalidateRect(hWnd, &rec, FALSE);

	prevX_ = x_;
	prevY_ = y_;
	prevRadius_ = radius_;
	
	x_ += 0.01;
	y_ -= 0.01;
//	radius_ -= 0.01;
}
