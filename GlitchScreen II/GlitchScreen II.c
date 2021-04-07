#include <Windows.h>
#define COLOR 3
#define COUNT 7
// 花屏程度
#define INTENSITY 20

int g_width = 0, g_height = 0;
HWND g_hwndWindow;
HDC g_hdc, g_hdcCopy;
HBITMAP g_hBitmapCopy;
HGDIOBJ g_hGdiobj;
// 红, 橙, 黄, 绿, 青, 蓝, 紫
COLORREF dwColor[COUNT][COLOR] = { {255, 0, 0}, {255, 128, 0}, {255, 255, 0}, {0, 255, 0}, {0, 255, 255}, {0, 0, 255}, {128, 0, 128} };

// 随机数函数
HCRYPTPROV hProv;
int Random()
{
	if (hProv == 0)
		if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_SILENT | CRYPT_VERIFYCONTEXT))
			return 0;

	int out = 0;
	CryptGenRandom(hProv, sizeof(out), (BYTE*)(&out));

	return out & 0x7FFFFFFF;
}

DWORD WINAPI HorizontalGlitch(LPVOID lpParameter)
{
	SIZE size;
	POINT ptStr;
	ZeroMemory(&ptStr, sizeof(ptStr));
	ZeroMemory(&size, sizeof(size));
	size.cx = g_width;
	size.cy = g_height;
	HDC hdcHori = CreateCompatibleDC(g_hdc);
	HBITMAP hBitmapHori = CreateCompatibleBitmap(g_hdc, 1, g_height);
	HGDIOBJ hGdiobj = SelectObject(hdcHori, hBitmapHori);

	int i = 0, ix = (Random() % 2) ? g_width - 1 : 0;
	int nPixel = Random() % COUNT;
	SetPixel(hdcHori, 0, 0, RGB(dwColor[nPixel][0], dwColor[nPixel][1], dwColor[nPixel][2]));
	StretchBlt(hdcHori, 0, 0, 1, g_height, hdcHori, 0, 0, 1, 1, SRCCOPY);
	for (i = (ix == 0) ? 1 : -1; i == -1 && ix > 0 || i == 1 && ix < g_width; ix += i)
	{
		Sleep(INTENSITY);
		if (Random() % INTENSITY)
			continue;

		BitBlt(g_hdcCopy, ix, 0, 1, g_height, hdcHori, 0, 0, SRCCOPY);
		//while (!BitBlt(g_hdcCopy, ix, 0, 1, g_height, hdcHori, 0, 0, SRCCOPY)) {}
		UpdateLayeredWindow(g_hwndWindow, NULL, NULL, &size, g_hdcCopy, &ptStr, RGB(0, 0, 0), NULL, ULW_COLORKEY);
	}
	SelectObject(hdcHori, hGdiobj);
	DeleteObject(hBitmapHori);
	DeleteDC(hdcHori);

	return 0;
}

DWORD WINAPI VerticalGlitch(LPVOID lpParameter)
{
	SIZE size;
	POINT ptStr;
	ZeroMemory(&ptStr, sizeof(ptStr));
	ZeroMemory(&size, sizeof(size));
	size.cx = g_width;
	size.cy = g_height;
	HDC hdcVert = CreateCompatibleDC(g_hdc);
	HBITMAP hBitmapVert = CreateCompatibleBitmap(g_hdc, g_width, 1);
	HGDIOBJ hGdiobj = SelectObject(hdcVert, hBitmapVert);

	int i = 0, iy = (Random() % 2) ? g_height - 1 : 0;
	int nPixel = Random() % COUNT;
	SetPixel(hdcVert, 0, 0, RGB(dwColor[nPixel][0], dwColor[nPixel][1], dwColor[nPixel][2]));
	StretchBlt(hdcVert, 0, 0, g_width, 1, hdcVert, 0, 0, 1, 1, SRCCOPY);
	for (i = (iy == 0) ? 1 : -1; i == -1 && iy > 0 || i == 1 && iy < g_height; iy += i)
	{
		Sleep(INTENSITY);
		if (Random() % INTENSITY)
			continue;

		BitBlt(g_hdcCopy, 0, iy, g_width, 1, hdcVert, 0, 0, SRCCOPY);
		//while (!BitBlt(g_hdcCopy, 0, iy, g_width, 1, hdcVert, 0, 0, SRCCOPY)) {}
		UpdateLayeredWindow(g_hwndWindow, NULL, NULL, &size, g_hdcCopy, &ptStr, RGB(0, 0, 0), NULL, ULW_COLORKEY);
	}
	SelectObject(hdcVert, hGdiobj);
	DeleteObject(hBitmapVert);
	DeleteDC(hdcVert);

	return 0;
}

DWORD WINAPI StartGlitch(LPVOID lpParameter)
{
	int delay = 0;
	g_width = GetSystemMetrics(SM_CXSCREEN);
	g_height = GetSystemMetrics(SM_CYSCREEN);
	g_hdc = GetDC(g_hwndWindow);
	g_hdcCopy = CreateCompatibleDC(g_hdc);
	g_hBitmapCopy = CreateCompatibleBitmap(g_hdc, g_width, g_height);
	g_hGdiobj = SelectObject(g_hdcCopy, g_hBitmapCopy);

	HANDLE hThread = NULL;
	for (;;)
	{
		if (Random() % 2)
			hThread = CreateThread(NULL, 0, HorizontalGlitch, NULL, 0, NULL);
		else
			hThread = CreateThread(NULL, 0, VerticalGlitch, NULL, 0, NULL);
		while ((delay = (Random() % 10000)) < 7777) {}
		Sleep(delay);
	}
	CloseHandle(hThread);
	SelectObject(g_hdcCopy, g_hGdiobj);
	DeleteObject(g_hBitmapCopy);
	DeleteDC(g_hdcCopy);
	ReleaseDC(g_hwndWindow, g_hdc);

	return 0;
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	// 设置进程的优先级为高
	SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

	// 注册窗口类
	WNDCLASSEX WinEx;
	WinEx.cbSize = sizeof(WinEx);
	WinEx.lpfnWndProc = (WNDPROC)DefWindowProc;
	WinEx.lpszClassName = "Glitch";
	WinEx.style = 0;
	WinEx.cbClsExtra = 0;
	WinEx.cbWndExtra = 0;
	WinEx.hInstance = GetModuleHandle(NULL);
	WinEx.hIcon = 0;
	WinEx.hCursor = LoadCursor(NULL, IDC_ARROW);
	WinEx.hbrBackground = NULL;
	WinEx.lpszMenuName = NULL;
	WinEx.hIconSm = 0;

	RegisterClassEx(&WinEx);

	// 创建一个置于顶层的透明窗口
	g_hwndWindow = CreateWindowEx(WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW, "Glitch", "", WS_POPUP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), NULL, NULL, hInstance, NULL);
	ShowWindow(g_hwndWindow, SW_SHOW);
	UpdateWindow(g_hwndWindow);
	CreateThread(NULL, 0, StartGlitch, NULL, 0, NULL);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}