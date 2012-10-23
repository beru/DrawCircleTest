#pragma once

#include <windows.h>
#include <gdiplus.h>

#pragma comment(lib, "Gdiplus.lib")

using namespace Gdiplus;

namespace {

class GDIPlusSetup
{
public:
	GDIPlusSetup()
	{
		GdiplusStartup(&gdiToken, &gdiSI, NULL);
	}
	
	~GDIPlusSetup()
	{
		GdiplusShutdown(gdiToken);
	}
	
private:
	GdiplusStartupInput gdiSI;
	ULONG_PTR           gdiToken;
} setup;

}
