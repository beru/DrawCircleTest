#pragma once

#include "targetver.h"
#include <windows.h>

void OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
void OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
void OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);
void OnMouseDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
void OnMouseUp(HWND hWnd, WPARAM wParam, LPARAM lParam);
void OnMouseMove(HWND hWnd, WPARAM wParam, LPARAM lParam);
void OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam);
