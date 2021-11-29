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

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
TCHAR szMaskWindowClass[MAX_LOADSTRING];

HWND hWnd = 0;									// window handle
HWND hWndMask = 0;								// Masking window handle
HBITMAP hScreenBitmap;							// Screen capture 
HBRUSH hBackgroundBrush;						// Background color brush
HCURSOR hDefaultCursor;
HCURSOR hSizeHorizCursor;
int x;
int y;
int prevX, prevY;
int width;
int height;
HRGN hRgn;
HRGN hRgnMask;

int screenWidth;
int screenHeight;
HDC hdcMemory;
HHOOK hMouseHook;
bool resizing = false;

// Forward declarations of functions included in this code module:
ATOM MyRegisterClass (HINSTANCE hInstance, TCHAR* szClassName, HBRUSH hBrush);
BOOL InitInstance (HINSTANCE, int);
HRGN CreateRectTopTwoCornersRoundedRegion (HDC hdc, int radius, int startX);
LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK MouseHookProc (int nCode, WPARAM wParam, LPARAM lParam);

int APIENTRY _tWinMain (HINSTANCE hInstance,
                        HINSTANCE hPrevInstance,
                        LPTSTR lpCmdLine,
                        int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_RESIZEREDRAW, szWindowClass, MAX_LOADSTRING);
	LoadString(hInstance, IDC_MASK_WINDOW, szMaskWindowClass, MAX_LOADSTRING);

	hBackgroundBrush = ::CreateSolidBrush(RGB(118, 182, 252));
	hDefaultCursor = ::LoadCursor(NULL, IDC_ARROW);
	hSizeHorizCursor = ::LoadCursor(NULL, IDC_SIZEWE);
	MyRegisterClass(hInstance, szWindowClass, hBackgroundBrush);
	MyRegisterClass(hInstance, szMaskWindowClass, (HBRUSH)::GetStockObject(NULL_BRUSH)); // transparent

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
ATOM MyRegisterClass (HINSTANCE hInstance, TCHAR* szClassName, HBRUSH hBrush)
{
	WNDCLASSEX wcex = { 0 };

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.lpszClassName = szClassName;
	wcex.hbrBackground = hBrush;
	wcex.lpfnWndProc = WndProc;
	wcex.hCursor = hDefaultCursor;
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.cbClsExtra	= 0;
	wcex.cbWndExtra	= 0;
	wcex.hInstance = hInstance;

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
	InitCaption(0, width);

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
					      0); // no extra param

	if (!hWnd)
	{
		return FALSE;
	}

	HDC hScreenDC = ::GetDC(NULL); // Get screen DC
	HDC hdc = ::GetDC(NULL);
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
	hRgn = CreateRectTopTwoCornersRoundedRegion(hdc, WINDOW_CORNER_RADIUS, 0);
	::ReleaseDC(hWnd, hdc);
	::SetWindowRgn(hWnd, hRgn, FALSE);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

//===================================================================================
HRGN CreateRectTopTwoCornersRoundedRegion (HDC hdc, int radius, int startX)
{
    // Bracket begin a path 
    BeginPath(hdc);

	::MoveToEx(hdc, startX, 0, NULL);

    ::ArcTo(hdc,
            startX, // enclosing rect.left
            0, // enclosing rect.top
            startX + radius * 2 - 1, // enclosing rect.right
            radius * 2 - 1, // enclosing rect.bottom
            startX + radius - 1, // start point.x
            0, // start point.y
            startX, // end point.x
            radius - 1); // end point.y
    ::LineTo(hdc,
             startX, // x
             height - 1); // y
    ::LineTo(hdc,
             startX + width - 1, // x
             height - 1); // y
    ::LineTo(hdc,
             startX + width - 1, // x
             radius - 1); // y

    ::ArcTo(hdc,
            startX + width - (radius * 2 - 1), // enclosing rect.left
            0, // enclosing rect.top
            startX + width - 1, // enclosing rect.right
            radius * 2 - 1, // enclosing rect.bottom
            startX + width - 1, // start point.x
            radius - 1, // start point.y
            startX + width - (radius - 1), // end point.x
            0); // end point.height
    ::LineTo(hdc,
             startX + radius - 1, // x
             0); // y

    // Bracket end a path 
    EndPath(hdc);

    return ::PathToRegion(hdc);
}

//===================================================================================
void OnPaint (HWND hwnd, HDC hdc)
{
	RECT rect;
	HDC hdcMemory2 = ::CreateCompatibleDC(hdc);
	HBITMAP hMemoryBitmap = 0;
	HBITMAP hOldBitmap = 0;

	if (hwnd == hWnd)
	{
		::GetClientRect(hWnd, &rect);

		// Create a bitmap big enough for our client rectangle.
		hMemoryBitmap = ::CreateCompatibleBitmap(hdc,
												 rect.right, // width
												 rect.bottom); // height
		hOldBitmap = (HBITMAP)::SelectObject(hdcMemory2, hMemoryBitmap);
		::SetBkMode(hdcMemory2, TRANSPARENT);

		rect.top = CAPTION_HEIGHT - 1;
		::FillRect(hdcMemory2, &rect, hBackgroundBrush);
		DrawCaption(hdcMemory2);

		::BitBlt(hdc,
				 0,
				 0,
				 rect.right,
				 rect.bottom,
				 hdcMemory2,
				 0,
				 0,
				 SRCCOPY);

		::SelectObject(hdcMemory2, hOldBitmap);
		::DeleteObject(hMemoryBitmap);

		if (resizing)
		{
			resizing = false;
			::ShowWindow(hWndMask, SW_HIDE);
		}
	}
	else if (hwnd == hWndMask)
	{
		::GetClientRect(hWndMask, &rect);
		
		// Create a bitmap big enough for our client rectangle.
		hMemoryBitmap = ::CreateCompatibleBitmap(hdc,
												 rect.right, // width
												 rect.bottom); // height
		hOldBitmap = (HBITMAP)::SelectObject(hdcMemory2, hMemoryBitmap);
		::SetBkMode(hdcMemory2, TRANSPARENT);
		::BitBlt(hdcMemory2,
				 0, // x
				 0, // y
				 rect.right, // width
				 rect.bottom, // height
				 hdcMemory,
				 0, // Src x
				 y, // Src y
				 SRCCOPY); // Copy
		/*rect.left = x;
		rect.right = x + width;
		rect.top = 0;
		rect.bottom = height;
		::FillRect(hdc, &rect, ::CreateSolidBrush(RGB(255, 255, 0)));*/
		::FillRgn(hdcMemory2, hRgnMask, hBackgroundBrush);
		//::FillRect(hdc, &rect, ::CreateSolidBrush(RGB(255, 255, 0)));

		// Draw caption
		BeginPath(hdcMemory2);

		::MoveToEx(hdcMemory2, x, 0, NULL);

		::ArcTo(hdcMemory2,
				x, // enclosing rect.left
				0, // enclosing rect.top
				x + WINDOW_CORNER_RADIUS * 2 - 1, // enclosing rect.right
				WINDOW_CORNER_RADIUS * 2 - 1, // enclosing rect.bottom
				x + WINDOW_CORNER_RADIUS - 1, // start point.x
				0, // start point.y
				x, // end point.x
				WINDOW_CORNER_RADIUS - 1); // end point.y
		::LineTo(hdcMemory2,
				 x, // x
				 CAPTION_HEIGHT); // y
		::LineTo(hdcMemory2,
				 x + width - 1, // x
				 CAPTION_HEIGHT); // y
		::LineTo(hdcMemory2,
				 x + width - 1, // x
				 WINDOW_CORNER_RADIUS - 1); // y

		::ArcTo(hdcMemory2,
				x + width - (WINDOW_CORNER_RADIUS * 2 - 1), // enclosing rect.left
				0, // enclosing rect.top
				x + width - 1, // enclosing rect.right
				WINDOW_CORNER_RADIUS * 2 - 1, // enclosing rect.bottom
				x + width - 1, // start point.x
				WINDOW_CORNER_RADIUS - 1, // start point.y
				x + width - (WINDOW_CORNER_RADIUS - 1), // end point.x
				0); // end point.height
		::LineTo(hdcMemory2,
				 x + WINDOW_CORNER_RADIUS - 1, // x
				 0); // y

		// Bracket end a path 
		EndPath(hdcMemory2);
		::SelectClipPath(hdcMemory2, RGN_COPY);
		DrawCaption(hdcMemory2);
		//::FillRect(hdcMemory2, &rect, ::CreateSolidBrush(RGB(255, 0, 0)));

		::BitBlt(hdc,
				 0,
				 0,
				 rect.right,
				 rect.bottom,
				 hdcMemory2,
				 0,
				 0,
				 SRCCOPY);

		::SelectObject(hdcMemory2, hOldBitmap);
		::DeleteObject(hMemoryBitmap);
	}
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
	RECT rect;
	PAINTSTRUCT ps;
	HDC hdc;
	//std::stringstream str;
	//str << message << std::endl;
	//::OutputDebugStringA(str.str().c_str());

	switch (message)
	{
		case WM_NCHITTEST:
		{
			int xx = GET_X_LPARAM(lParam) - x;
			int yy = GET_Y_LPARAM(lParam) - y;

			if (yy <= CAPTION_HEIGHT)
			{
				return HTCAPTION;
			}

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
				resizing = true;
				::GetWindowRect(hWnd, &rect);
				x = rect.left;
				y = rect.top;
				width = rect.right - rect.left;
				height = rect.bottom - rect.top;
				int bitmapX = 0;
				int bitmapY = y;
				int bitmapWidth = x + width;
				int bitmapHeight = height;
				InitCaption(x, width);

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
				else
				{
					::MoveWindow(hWndMask,
								 bitmapX,
								 bitmapY,
								 bitmapWidth,
								 bitmapHeight,
								 FALSE);
				}

				HDC hdc = ::GetDC(hWndMask);
				hRgnMask = CreateRectTopTwoCornersRoundedRegion(hdc, WINDOW_CORNER_RADIUS, rect.left);
				::ReleaseDC(hWndMask, hdc);

				::ShowWindow(hWndMask, SW_SHOW);
				::UpdateWindow(hWndMask);
				hMouseHook = ::SetWindowsHookExA(WH_MOUSE_LL, MouseHookProc, 0, 0);

				return 0;
			}

			return DefWindowProc(hwnd, message, wParam, lParam);
		}
		case WM_WINDOWPOSCHANGED:
		{
			if (hwnd == hWnd && !resizing)
			{
				WINDOWPOS* winPos = (WINDOWPOS*)lParam;
				x = winPos->x;
				y = winPos->y;
			}

			return DefWindowProc(hwnd, message, wParam, lParam);
		}
		case WM_ERASEBKGND:
		{
			return 1;
		}
		case WM_PAINT:
		{
			hdc = BeginPaint(hwnd, &ps);
			OnPaint(hwnd, hdc);
			EndPaint(hwnd, &ps);
			break;
		}
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			break;
		}
		default:
		{
			return DefWindowProc(hwnd, message, wParam, lParam);
		}
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
					//::SetCursor(hSizeHorizCursor);
					int xx = mouseHookInfo->pt.x;
					width -= (xx - x);
					int minX = min(x, xx);
					int maxX = max(x, xx);
					
					rect.left = minX - WINDOW_CORNER_RADIUS;
					rect.right = maxX + WINDOW_CORNER_RADIUS;
					rect.top = 0;
					rect.bottom = height;
					
					x = xx;

					HDC hdc = ::GetDC(hWndMask);
					::DeleteRgn(hRgnMask);
					hRgnMask = CreateRectTopTwoCornersRoundedRegion(hdc, WINDOW_CORNER_RADIUS, x);
					::ReleaseDC(hWndMask, hdc);
					InitCaption(x, width);
					
					std::stringstream str;
					str << "X = " << x << ", width = " << width << std::endl;
					::OutputDebugStringA(str.str().c_str());
					::InvalidateRect(hWndMask, &rect, FALSE);

					rect.right = x + width;
					rect.bottom = CAPTION_HEIGHT;
					::GetClientRect(hWndMask, &rect);
					::InvalidateRect(hWndMask, &rect, FALSE);
				}
            }
			else if (wParam == WM_LBUTTONUP)
			{
				InitCaption(0, width);
				HDC hdc = ::GetDC(hWnd);
				::DeleteRgn(hRgn);
				hRgn = CreateRectTopTwoCornersRoundedRegion(hdc, WINDOW_CORNER_RADIUS, 0);
				::ReleaseDC(hWnd, hdc);
				::SetWindowRgn(hWnd, hRgn, FALSE);
				::MoveWindow(hWnd, x, y, width, height, TRUE);
				::UnhookWindowsHookEx(hMouseHook);
				::SetCursor(hDefaultCursor);
				//resizing = false;
				//::Sleep(500);
				//::ShowWindow(hWndMask, SW_HIDE);
				//::SetActiveWindow(hWnd);
			}
        }
    }

    return ::CallNextHookEx(0, nCode, wParam, lParam);
}
