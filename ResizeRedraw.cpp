// ResizeRedraw.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "ResizeRedraw.h"

#include <sstream>

#define MAX_LOADSTRING 100
#define WINDOW_X 200
#define WINDOW_Y 200
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define WINDOW_BORDER_WIDTH 6
#define WINDOW_CORNER_RADIUS 8
#define CAPTION_HEIGHT 50

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

HWND hWnd = 0;									// window handle
HWND hWndMask = 0;								// Masking window handle
HBITMAP hScreenBitmap;							// Screen capture 
HBRUSH hBackgroundBrush;						// Background color brush
HCURSOR hDefaultCursor;
int x;
int y;
int prevX, prevY;
int width;
int height;

int screenWidth;
int screenHeight;
HDC hdcMemory;
HHOOK hMouseHook;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK MouseHookProc (int nCode, WPARAM wParam, LPARAM lParam);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_RESIZEREDRAW, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_RESIZEREDRAW));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	hBackgroundBrush = ::CreateSolidBrush(RGB(118, 182, 252));
	hDefaultCursor = ::LoadCursor(NULL, IDC_ARROW);

	WNDCLASSEX wcex = { 0 };

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.lpszClassName	= szWindowClass;
	wcex.hbrBackground =  ::CreateSolidBrush(RGB(0, 0, 0));//(HBRUSH)::GetStockObject(NULL_BRUSH); // transparent
	wcex.lpfnWndProc	= WndProc;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance (HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable
	x = WINDOW_X;
	y = WINDOW_Y;
	width = WINDOW_WIDTH;
	height = WINDOW_HEIGHT;
	screenWidth = ::GetSystemMetrics(SM_CXFULLSCREEN);
	screenHeight = ::GetSystemMetrics(SM_CYFULLSCREEN);
	HDC hScreenDC = ::GetDC(NULL); // Get screen DC
	HDC hdc = ::GetDC(hWnd);
	hdcMemory = ::CreateCompatibleDC(hdc);
	hScreenBitmap = ::CreateCompatibleBitmap(hdc, screenWidth, screenHeight);
	::SelectObject(hdcMemory, hScreenBitmap);
	::BitBlt(hdcMemory,
			 0,
			 0,
			 screenWidth,
			 screenHeight,
			 hScreenDC,
			 0,
			 0,
			 SRCCOPY);

   hWnd = ::CreateWindow(szWindowClass,
					     0, // no title
					     WS_POPUP,
					     x,
					     y,
					     width,
					     height,
					     NULL, // no parent window
					     NULL, // no menu
					     hInstance,
					     0);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//===================================================================================
void OnPaint (HWND hwnd, HDC hdc)
{
	RECT rect;
    ::GetClientRect(hwnd, &rect);

	if (hwnd == hWnd)
	{
		::FillRect(hdc, &rect, hBackgroundBrush);
	}
	else if (hwnd == hWndMask)
	{
		::BitBlt(hdc,
				 0, // x
				 0, // y
				 rect.right, // width
				 rect.bottom, // height
				 hdcMemory,
				 0, // Src x
				 y, // Src y
				 SRCCOPY); // Copy
		rect.left = x;
		rect.right = x + width;
		rect.top = 0;
		rect.bottom = height;
		::FillRect(hdc, &rect, ::CreateSolidBrush(RGB(255, 255, 0)));
	}
	else
	{
		::FillRect(hdc, &rect, ::CreateSolidBrush(RGB(0, 255, 0)));
	}
	//DrawBorder(hdc);
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	std::stringstream str;
	str << message << std::endl;
	::OutputDebugStringA(str.str().c_str());

	switch (message)
	{
	case WM_NCHITTEST:
    {
		int xx = GET_X_LPARAM(lParam) - x;
        int yy = GET_Y_LPARAM(lParam) - y;

		if (xx <= WINDOW_BORDER_WIDTH)
		{
			return HTLEFT;
		}

        return HTCLIENT;
    }
	case WM_SYSCOMMAND:
	{
		if (wParam == SC_SZLEFT)
		{
			prevX = x;
			prevY = y;
			
			int bitmapX = 0;
			int bitmapY = y;
			int bitmapWidth = x + width;
			int bitmapHeight = height;

			if (!hWndMask)
			{
				hWndMask = ::CreateWindowEx(WS_EX_TOPMOST,
										    szWindowClass,
											0, // No title
											WS_POPUP,
											bitmapX,
											bitmapY,
											bitmapWidth,
											bitmapHeight,
											NULL, // no parent
											NULL, // no menu
											hInst,
											0); // no param
			}

			::ShowWindow(hWndMask, SW_SHOW);
			::UpdateWindow(hWndMask);
			hMouseHook = ::SetWindowsHookExA(WH_MOUSE_LL, MouseHookProc, 0, 0);

			return 0;
		}

		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		OnPaint(hwnd, hdc);
		EndPaint(hwnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	return 0;
}

//===================================================================================
LRESULT CALLBACK MouseHookProc (int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode < 0)
    {
        return ::CallNextHookEx(0, nCode, wParam, lParam);
    }

    if (nCode == HC_ACTION)
    {
		static RECT rect;
		::GetWindowRect(hWndMask, &rect);

        MOUSEHOOKSTRUCT* mouseHookInfo = (MOUSEHOOKSTRUCT*)lParam;

        if (mouseHookInfo)
        {
            if (wParam == WM_MOUSEMOVE)
            {
                if (::PtInRect(&rect, mouseHookInfo->pt))
                {
					//::OutputDebugStringA("Mouse Move\n");
					
					int xx = mouseHookInfo->pt.x;
					width -= (xx - x);
					int minX = min(x, xx);
					int maxX = max(x, xx);
					RECT rect;
					rect.left = minX - 1;
					rect.right = maxX + 1;
					rect.top = 0;
					rect.bottom = height;
					std::stringstream str;
					x = xx;
					str << "X = " << x << std::endl;
					::OutputDebugStringA(str.str().c_str());
					::InvalidateRect(hWndMask, &rect, FALSE);
				}
            }
			else if (wParam == WM_LBUTTONUP)
			{
				::MoveWindow(hWnd, x, y, width, height, TRUE);
				::UnhookWindowsHookEx(hMouseHook);
				::SetCursor(hDefaultCursor);
				::ShowWindow(hWndMask, SW_HIDE);
			}
        }
    }

    return ::CallNextHookEx(0, nCode, wParam, lParam);
}
