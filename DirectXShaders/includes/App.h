#pragma once
#include <Windows.h>

const UINT WINDOW_WIDTH = 1920;
const UINT WINDOW_HEIGHT = 1080;

extern float g_DeltaTime;
extern float g_LastFrame;

extern bool g_KeyStates[256];

void StartApp(const TCHAR* appName);
void ProcessInput(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);