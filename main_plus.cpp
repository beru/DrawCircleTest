#include "stdafx.h"

#include "main_plus.h"
#include <assert.h>

#include "stdint.h"
#include <vector>

#include "mg/renderer24.h"
#include "mg/container.h"
#include "mg/button.h"

#include "winutil.h"
#include "arrayutil.h"
#include "ReadImage/File.h"
#include "ReadImage/ReadImage.h"

namespace MG {

struct BasicBG : public Element::IBackground
{
public:
	optional<Color> color;
	Image image;
	bool repeatX;
	bool repeatY;

	virtual void Draw(const Rectangle& rect, IRenderer& renderer)
	{
		if (color) {
			Color c = *color;
			renderer.FillRectangle(rect, c);
		}
		if (image.pBitmap) {
			const Bitmap& bmp = *image.pBitmap;
			renderer.DrawBitmap(bmp, image.GetRect(), rect);
		}
	}
};

} // namespace MG

namespace {

HWND hWnd;
HBITMAP hBMP;
std::vector<uint8_t> bmiBuff(sizeof(BITMAPINFO) + sizeof(RGBQUAD) * 256);
//BITMAPINFO bmi;
void* pBits;
HDC hMemDC;

MG::Renderer24 renderer;

MG::Container mg(4, 8);
MG::Button button1(4);
MG::Button button2(0);
MG::Element pic1(0);
MG::Element pic2(0);

MG::Bitmap bmp24Content;
std::vector<uint8_t> image24Buffer;

MG::Bitmap bmp32Content;
std::vector<uint8_t> image32Buffer;

MG::BasicBG mainbg;
MG::BasicBG buttonBG;
MG::BasicBG buttonBGPressed;
MG::BasicBG picbg1;
MG::BasicBG picbg2;

bool ReadImage(const char* filename, MG::Bitmap& bmp, std::vector<uint8_t>& buffer)
{
	FILE* file = fopen(filename, "r");
	if (!file) {
		return false;
	}
	File fileProxy(file);
	ImageInfo imageInfo;
	ReadImageInfo(fileProxy, imageInfo);
	assert(imageInfo.bitsPerSample == 8);
	assert(imageInfo.samplesPerPixel == 3 || imageInfo.samplesPerPixel == 4);
	buffer.resize(imageInfo.width * imageInfo.height * imageInfo.samplesPerPixel);
	int lineOffsetBytes = imageInfo.width * imageInfo.samplesPerPixel;
	ReadImageData(fileProxy, &buffer[0], lineOffsetBytes, 0);
	fclose(file);
	bmp.bitsPerPixel = imageInfo.bitsPerSample * imageInfo.samplesPerPixel;
	bmp.width = imageInfo.width;
	bmp.height = imageInfo.height;
	bmp.lineOffsetBytes = lineOffsetBytes;
	bmp.pBits = &buffer[0];
	return true;
}

} // anonymous namespace

void OnButton1MouseDown(void* param, const MG::Element::Event& e)
{
	OutputDebugString(L"MouseDown\n");
}

void OnButton1MouseUp(void* param, const MG::Element::Event& e)
{
	OutputDebugString(L"MouseUp\n");
}

void OnButton1MouseMove(void* param, const MG::Element::Event& e)
{
	OutputDebugString(L"MouseMove\n");
}

void OnButton1Click(void* param, const MG::Element::Event& e)
{
	OutputDebugString(L"Click\n");
}

void OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	BITMAPINFO* pBMI = (BITMAPINFO*) &bmiBuff[0];
	BITMAPINFO& bmi = *pBMI;
	
	int width = 640;
	int height = 480;
	int bitsPerPixel = 24;
	
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
	
	MG::Bitmap bmp;
	bmp.bitsPerPixel = bitsPerPixel;
	bmp.width = width;
	bmp.height = height;
	bmp.pBits = pBits;
	bmp.lineOffsetBytes = width * 3;
	renderer.SetBitmap(bmp);
	
	MG::Rectangle rect;
	rect.x = 0;
	rect.y = 0;
	rect.w = width;
	rect.h = height;
	mg.rect_ = rect;
	MG::Color col;
	col.r = 50;
	col.g = 200;
	col.b = 150;
	
	mainbg.color = col;
	mg.pBG_ = &mainbg;
	mg.needsToDraw_ = true;
	
	col.g = 100;
	buttonBG.color = col;
	col.r = 200;
	buttonBGPressed.color = col;
	
	rect.x = 100;
	rect.y = 100;
	rect.w = 100;
	rect.h = 50;
	button1.rect_ = rect;
	button1.pBG_ = &buttonBG;
	button1.pBGPressed_ = &buttonBGPressed;
	mg.AddChild(button1);
	
	rect.x = 300;
	rect.y = 100;
	rect.w = 100;
	rect.h = 50;
	button2.rect_ = rect;
	button2.pBG_ = &buttonBG;
	button2.pBGPressed_ = &buttonBGPressed;
	mg.AddChild(button2);
	
	if (!ReadImage("test24.bmp", bmp24Content, image24Buffer)) {
		return;
	}
	if (!ReadImage("test32.bmp", bmp32Content, image32Buffer)) {
		return;
	}
	
	rect.x = 100;
	rect.y = 300;
	pic1.rect_ = rect;
	MG::Rectangle irec;
	irec.x = 510;
	irec.y = 340;
	irec.w = 400;
	irec.h = 400;
	picbg1.image.rect = irec;
	picbg1.image.pBitmap = &bmp24Content;
	pic1.pBG_ = &picbg1;
	pic1.needsToDraw_ = true;
	mg.AddChild(pic1);
	
	rect.x = 300;
	rect.y = 200;
	rect.w = 336;
	rect.h = 300;
	pic2.rect_ = rect;
	irec.x = 0;
	irec.y = 0;
	irec.w = 336;
	irec.h = 24;
	picbg2.image.rect = irec;
	picbg2.image.pBitmap = &bmp32Content;
	pic2.pBG_ = &picbg2;
	pic2.needsToDraw_ = true;
	mg.AddChild(pic2);
	
	HDC hWndDC = ::GetDC(hWnd);
	hMemDC = ::CreateCompatibleDC(hWndDC);
	::SetMapMode(hMemDC, ::GetMapMode(hWndDC));
	::ReleaseDC(hWnd, hWndDC);
	::SelectObject(hMemDC, hBMP);

	button1.AddEventListener(MG::Element::EventType_MouseDown, OnButton1MouseDown);
	button1.AddEventListener(MG::Element::EventType_MouseUp, OnButton1MouseUp);
//	button1.AddEventListener(MG::Element::EventType_MouseMove, OnButton1MouseMove);
	button1.AddEventListener(MG::Button::EventType_Click, OnButton1Click);
	

	mg.Draw(0, 0, renderer);
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
		mg.OnMouseDown(lParam & 0xFFFF, lParam >> 16);
//	}
}

void OnMouseUp(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
//	if (wParam & MK_LBUTTON) {
		mg.OnMouseUp(lParam & 0xFFFF, lParam >> 16);
//	}
}

void OnMouseMove(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
//	if (wParam & MK_LBUTTON) {
		mg.OnMouseMove(lParam & 0xFFFF, lParam >> 16);
//	}
}

void OnTime(HWND hWnd)
{
	mg.Draw(0, 0, renderer);
	MG::Rectangle updated = renderer.GetUpdatedRect();
	if (updated.w && updated.h) {
		renderer.ClearUpdatedRect();
		RECT r;
		r.left = updated.x;
		r.right = r.left + updated.w;
		r.top = updated.y;
		r.bottom = r.top + updated.h;
		::InvalidateRect(hWnd, &r, FALSE);
	}
}
