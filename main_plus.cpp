

#include "main_plus.h"
#include <assert.h>

#include "stdint.h"
#include <vector>

#include "winutil.h"
#include "arrayutil.h"

#include "graphics/graphics.h"
#include <tchar.h>
#include <stdio.h>

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

void OnMouseMove(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
//	if (wParam & MK_LBUTTON) {
//	}
}

void OnTime(HWND hWnd)
{
	if (!IsWindow(hWnd)) {
		return;
	}
	static float x = 601.5;
	static float y = 300.0;
	static float radius = 300;
	float diameter = radius*2;
	
	

	RECT rec;
	rec.left = x-radius-1;
	rec.top = y-radius-1;
	rec.right = rec.left + diameter+10;
	rec.bottom = rec.top + diameter+10;
	Graphics::FillRect(rec.left, rec.top, rec.right-rec.left, rec.bottom-rec.top, 0);

	DWORD st = ::timeGetTime();

	Graphics::DrawFilledCircleAA2(x, y, diameter, 0x00FF00);

	DWORD et = ::timeGetTime();

	::InvalidateRect(hWnd, &rec, FALSE);

	TCHAR str[32];
	_stprintf(str, _T("%d"), et-st);
	::SetTextColor(hMemDC, -1);
	::TextOut(hMemDC, 10, 10, str, _tcslen(str));
	rec.left = 10;
	rec.top = 10;
	rec.right = 100;
	rec.bottom = 100;
	::InvalidateRect(hWnd, &rec, FALSE);

	x += 0.01;
	y += 0.01;
	radius += 0.01;
}
