#include <thread>
#include <iostream>

#include "memory.h"
#include "offsets.h"

#define EnemyPen 0x000000FF
HBRUSH EnemyBrush = CreateSolidBrush(0x000000FF);

int width = 1920;
int height = 1080;

struct view_matrix_t {
	float* operator[ ](int index) {
		return matrix[index];
	}

	float matrix[4][4];
};

struct Vector3
{
	float x, y, z;
};

Vector3 WorldToScreen(const Vector3 pos, view_matrix_t matrix) {
	float _x = matrix[0][0] * pos.x + matrix[0][1] * pos.y + matrix[0][2] * pos.z + matrix[0][3];
	float _y = matrix[1][0] * pos.x + matrix[1][1] * pos.y + matrix[1][2] * pos.z + matrix[1][3];

	float w = matrix[3][0] * pos.x + matrix[3][1] * pos.y + matrix[3][2] * pos.z + matrix[3][3];

	float inv_w = 1.f / w;
	_x *= inv_w;
	_y *= inv_w;

	float x = width * .5f;
	float y = height * .5f;

	x += 0.5f * _x * width + 0.5f;
	y -= 0.5f * _y * height + 0.5f;

	return { x,y,w };
}

void DrawBorderBox(HDC hdc, int x, int y, int w, int h)
{
	Rectangle(hdc, x, y, w, h);
}

void DrawLine(HDC hdc, float StartX, float StartY, float EndX, float EndY)
{
	int a, b = 0;
	HPEN hOPen;
	HPEN hNPen = CreatePen(PS_SOLID, 2, EnemyPen);// penstyle, width, color
	hOPen = (HPEN)SelectObject(hdc, hNPen);
	MoveToEx(hdc, StartX, StartY, NULL); //start
	a = LineTo(hdc, EndX, EndY); //end
	DeleteObject(SelectObject(hdc, hOPen));
}


void draw(HDC hdc) {
	std::this_thread::sleep_for(std::chrono::milliseconds(1));
	

	
	uintptr_t localPlayer = VARS::memRead<uintptr_t>(VARS::baseAddress + offsets::dwLocalPlayer);
	

	view_matrix_t vm = VARS::memRead<view_matrix_t>(VARS::baseAddress + offsets::dwViewMatrix);
	int localteam = VARS::memRead<int>(VARS::memRead<DWORD>(VARS::baseAddress + offsets::dwLocalPlayer) + offsets::m_iTeamNum);

	for (int i = 1; i < 64; i++)
	{
		
		uintptr_t pEnt = VARS::memRead<DWORD>(VARS::baseAddress + offsets::dwEntityList + (i * 0x10));
		int health = VARS::memRead<int>(pEnt + offsets::m_iHealth);
		int team = VARS::memRead<int>(pEnt + offsets::m_iTeamNum);

		if (!localPlayer) {
			continue;
		}
		if (!pEnt) {
			continue;
		}
		if (VARS::memRead<int>(localPlayer + offsets::m_iHealth) < 0 || VARS::memRead<int>(localPlayer + offsets::m_iHealth) > 100) {
			continue;
		}

		Vector3 pos = VARS::memRead<Vector3>(pEnt + offsets::m_vecOrigin);
		Vector3 head;
		head.x = pos.x;
		head.y = pos.y;
		head.z = pos.z + 75.f;
		Vector3 screenpos = WorldToScreen(pos, vm);
		Vector3 screenhead = WorldToScreen(head, vm);
		float heights = screenhead.y - screenpos.y;
		float widths = height / 2.4f;

		if (screenpos.z >= 0.01f && team != localteam && health > 0 && health < 101) {
			DrawBorderBox(hdc, screenhead.x - 25, screenhead.y - 10, screenpos.x + 10, screenpos.y);
			//DrawSquare(hdc, screenpos.x, screenpos.y, 10);
			DrawLine(hdc, width / 2, height, screenpos.x, screenpos.y);
		}
	}
}



LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HDC hdcBuffer = NULL;
	static HBITMAP hbmBuffer = NULL;
	switch (message)
	{
		

	case WM_CREATE:
	{

		hdcBuffer = CreateCompatibleDC(NULL);
		hbmBuffer = CreateCompatibleBitmap(GetDC(hWnd), width, height);
		SelectObject(hdcBuffer, hbmBuffer);

		// Set the window style to a layered window with an alpha channel
		SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);

		// Make the window 100% transparent (0 alpha)
		SetLayeredWindowAttributes(hWnd, RGB(255, 255, 255), 0, LWA_COLORKEY);


		break;
	}

	//a way to handle the WM_ERASEBKGND message is to return TRUE from the window procedure, indicating that the background has been erased and there's no need for the system to perform additional background erasure.
	case WM_ERASEBKGND:
		// We handle this message to avoid flickering
		return TRUE;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

		/*
		normal screen refreshing no double buffering
		SetBkMode(hdc, TRANSPARENT);
		HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255));
		FillRect(hdc, &ps.rcPaint, hBrush);

		DeleteObject(hBrush);

		*/
		
		//DOUBLE BUFFERING
		FillRect(hdcBuffer, &ps.rcPaint, (HBRUSH)GetStockObject(WHITE_BRUSH));
		
		draw(hdcBuffer);
		BitBlt(hdc, 0, 0, width, height, hdcBuffer, 0, 0, SRCCOPY);


		EndPaint(hWnd, &ps);
		InvalidateRect(hWnd, NULL, TRUE);
		break;
	}
	case WM_DESTROY:
		DeleteDC(hdcBuffer);
		DeleteObject(hbmBuffer);
	
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// Register the window class
	const char CLASS_NAME[] = "SimpleWindowClass";

	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpfnWndProc = WndProc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;
	
	RegisterClassEx(&wc);

	// Create the window
	HWND hWnd = CreateWindowEx(0, CLASS_NAME, "Simple Window with GDI", WS_EX_TRANSPARENT | WS_EX_TOPMOST,
		CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, hInstance, NULL);

	if (hWnd == NULL)
		return 0;

	SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	ShowWindow(hWnd, nCmdShow);

	// Message loop
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}







