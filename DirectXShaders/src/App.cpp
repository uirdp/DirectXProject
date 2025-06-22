#include "App.h"
#include "Engine.h"
#include "Scene.h"
#include "Timer.h"
#include <stdio.h>
#include <windowsx.h>

HINSTANCE g_hInst;
HWND g_hWnd = NULL;

float g_DeltaTime = 0.0f;
float g_LastFrame = 0.0f; 

bool g_KeyStates[256] = { false };

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{

	case WM_SETFOCUS: // ウィンドウがフォーカスを得たとき
		ShowCursor(FALSE); // カーソルを非表示
		break;

	case WM_KILLFOCUS: // ウィンドウがフォーカスを失ったとき
		ShowCursor(TRUE); // カーソルを再表示
		break;

	case WM_KEYDOWN:
		if(wParam == 27) PostQuitMessage(0); // escape
		g_KeyStates[wParam] = true;
		break;

	case WM_KEYUP:
		g_KeyStates[wParam] = false;
		break;

	case VK_ESCAPE: // エスケープキーが押された場合
		printf("esc\n");
		PostQuitMessage(0); // アプリケーションを終了
		break;

	case WM_MOUSEMOVE:
	{
		RECT rect;
		GetClientRect(hWnd, &rect);
		//　画面の中央を取得
		POINT center = { (rect.right - rect.left) / 2, (rect.bottom - rect.top) / 2 };
		ClientToScreen(hWnd, &center);

		// 現在のマウスの位置
		POINT currentPos;
		GetCursorPos(&currentPos);

		// マウスの移動量
		int xOffset = currentPos.x - center.x;
		int yOffset = currentPos.y - center.y;

		g_Scene->ProcessMouseMovement(xOffset, yOffset);

		SetCursorPos(center.x, center.y);
		break;
	}
	

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

void InitWindow(const TCHAR* appName)
{
	g_hInst = GetModuleHandle(nullptr);
	if (g_hInst == nullptr)
	{
		MessageBox(nullptr, L"GetModuleHandle failed", L"Error", MB_OK);
		return;
	}

	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hIcon = LoadIcon(g_hInst, IDI_APPLICATION);
	wc.hCursor = LoadCursor(g_hInst, IDC_ARROW);
	wc.hbrBackground = GetSysColorBrush(COLOR_BACKGROUND);
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = appName;
	wc.hIconSm = LoadIcon(g_hInst, IDI_APPLICATION);

	RegisterClassEx(&wc);

	RECT rect = {};
	rect.right = static_cast<LONG>(WINDOW_WIDTH);
	rect.bottom = static_cast<LONG>(WINDOW_HEIGHT);

	auto style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
	AdjustWindowRect(&rect, style, FALSE);

	g_hWnd = CreateWindowEx(0, appName, appName, style, CW_USEDEFAULT, CW_USEDEFAULT,
		rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, g_hInst, nullptr);

	ShowWindow(g_hWnd, SW_SHOWNORMAL);

	SetFocus(g_hWnd);
}

void EnableDebugLayer()
{
	ComPtr<ID3D12Debug> debugLayer;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(debugLayer.GetAddressOf()))))
	{
		debugLayer->EnableDebugLayer();
		debugLayer->Release();
	}

}

void MainLoop()
{
	Timer timer;
	const float targetFrameRate = (1.0f / 60.0f) * 1000.0f;

	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE == TRUE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			float currentTime = timer.GetElapsedTime();
			g_DeltaTime = currentTime - g_LastFrame;
			g_LastFrame = currentTime;

			if (g_DeltaTime < targetFrameRate)
			{
				Sleep(static_cast<DWORD>((targetFrameRate - g_DeltaTime)));
				g_DeltaTime = targetFrameRate;
			}

			g_Scene->Update();
			g_Engine->BeginRender();
			g_Scene->Draw();
			g_Engine->EndRender();
			g_Engine->UpdateFrameCount();
		}
	}
}

void StartApp(const TCHAR* appName)
{
	InitWindow(appName);

#ifdef _DEBUG
	EnableDebugLayer();
#endif


	g_Engine = new Engine();
	if (!g_Engine->Init(g_hWnd, WINDOW_WIDTH, WINDOW_HEIGHT))
	{
		MessageBox(nullptr, L"Engine initialization failed", L"Error", MB_OK);
		return;
	}
	g_Engine->InitIrradianceMap();

	g_Scene = new Scene();
	if (!g_Scene->Init())
	{
		MessageBox(nullptr, L"Scene initialization failed", L"Error", MB_OK);
		return;
	}

	g_Engine->DrawIrradianceMap();

	MainLoop();
}

void ProcessInput(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (wParam)
    {
		case WM_KEYDOWN:
			g_KeyStates[wParam] = true;
			break;

		case WM_KEYUP:
			g_KeyStates[wParam] = false;
			break;

		case VK_ESCAPE: // エスケープキーが押された場合
			printf("esc\n");
			PostQuitMessage(0); // アプリケーションを終了
			break;

		default:
		break;
    }
}
