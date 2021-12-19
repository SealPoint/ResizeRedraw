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
HBRUSH hBackgroundBrush;						// Background color brush
HCURSOR hDefaultCursor;
HCURSOR hSizeHorizCursor;
int x;
int y;
int prevX, prevY;
int width;
int height;
HRGN hRgn = 0;
HFONT hTitleFont;
HBITMAP hCaptionBitmap;

int screenWidth;
int screenHeight;
HDC hdcMemory;
HHOOK hMouseHook;
bool resizing = false;

// Forward declarations of functions included in this code module:
ATOM MyRegisterClass (HINSTANCE hInstance, TCHAR* szClassName, HBRUSH hBrush);
BOOL InitInstance (HINSTANCE, int);
HRGN CreateRectTopTwoCornersRoundedRegion (HDC hdc, int radius, int startX, int startY);
static BOOL CALLBACK EnumWindowCallback (HWND wAppWnd, LPARAM lParam);
LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK MouseHookProc (int nCode, WPARAM wParam, LPARAM lParam);

int APIENTRY _tWinMain (HINSTANCE hInstance,
                        HINSTANCE hPrevInstance,
                        LPTSTR lpCmdLine,
                        int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	//::EnumWindows(EnumWindowCallback, NULL);

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
	hCaptionBitmap = (HBITMAP)::LoadImage(hInstance, MAKEINTRESOURCE(IDB_STAR), IMAGE_BITMAP, 0, 0, 0);

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
	wcex.hbrBackground = (HBRUSH)::GetStockObject(NULL_BRUSH);
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
					      0,
					      0,
					      screenWidth,
					      screenHeight,
					      NULL, // no parent window
					      NULL, // no menu
					      hInstance,
					      0); // no extra param

	if (!hWnd)
	{
		return FALSE;
	}

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

//===================================================================================
HRGN CreateRectTopTwoCornersRoundedRegion (HDC hdc, int radius, int startX, int startY)
{
    // Bracket begin a path 
    BeginPath(hdc);

	::MoveToEx(hdc, startX, startY, NULL);

    ::ArcTo(hdc,
            startX, // enclosing rect.left
            startY, // enclosing rect.top
            startX + radius * 2 - 1, // enclosing rect.right
            startY + radius * 2 - 1, // enclosing rect.bottom
            startX + radius - 1, // start point.x
            startY, // start point.y
            startX, // end point.x
            startY + radius - 1); // end point.y
    ::LineTo(hdc,
             startX, // x
             startY + height - 1); // y
    ::LineTo(hdc,
             startX + width - 1, // x
             startY + height - 1); // y
    ::LineTo(hdc,
             startX + width - 1, // x
             startY + radius - 1); // y

    ::ArcTo(hdc,
            startX + width - (radius * 2 - 1), // enclosing rect.left
            startY, // enclosing rect.top
            startX + width - 1, // enclosing rect.right
            startY + radius * 2 - 1, // enclosing rect.bottom
            startX + width - 1, // start point.x
            startY + radius - 1, // start point.y
            startX + width - (radius - 1), // end point.x
            startY); // end point.height
    ::LineTo(hdc,
             startX + radius - 1, // x
             startY); // y

    // Bracket end a path 
    EndPath(hdc);

    return ::PathToRegion(hdc);
}

//===================================================================================
void OnPaint (HWND hwnd, HDC hdc)
{
	if (!hRgn)
	{
		hRgn = CreateRectTopTwoCornersRoundedRegion(hdc, WINDOW_CORNER_RADIUS, x, y);
	}

	::SetBkMode(hdc, TRANSPARENT);
	::FillRgn(hdc, hRgn, hBackgroundBrush);

	/*HDC hdcMemory = ::CreateCompatibleDC(hdc);
	HBITMAP hMemoryBitmap = 0;
	HBITMAP hOldBitmap = 0;

	// Create a bitmap big enough for our client rectangle.
	hMemoryBitmap = ::CreateCompatibleBitmap(hdc,
											 width, // width
											 height); // height
	hOldBitmap = (HBITMAP)::SelectObject(hdcMemory, hMemoryBitmap);
	::SetBkMode(hdcMemory, TRANSPARENT);

	::FillRgn(hdcMemory, hRgn, hBackgroundBrush);

	//rect.top = CAPTION_HEIGHT - 1;
	//::FillRect(hdcMemory2, &rect, hBackgroundBrush);
	//DrawCaption(hdcMemory2, hCaptionBitmap);

	::BitBlt(hdc,
			 x,
			 y,
			 width,
			 height,
			 hdcMemory,
			 0,
			 0,
			 SRCCOPY);

	::SelectObject(hdcMemory, hOldBitmap);
	::DeleteObject(hMemoryBitmap);*/
}

static BOOL CALLBACK EnumWindowCallback (HWND wAppWnd, LPARAM lParam)
{
	int length = GetWindowTextLength(wAppWnd);
    char* buffer = new char[length + 1];
	::GetWindowTextA(wAppWnd, buffer, length + 1);

    // List visible windows with a non-empty title
	if (::IsWindowVisible(wAppWnd) && length > 0)
	{
		std::stringstream str;
		str << buffer << std::endl;
		::OutputDebugStr(str.str().c_str());
    }

	delete [] buffer;

    return TRUE;
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
	PAINTSTRUCT ps;
	HDC hdc;
	//std::stringstream str;
	//str << message << std::endl;
	//::OutputDebugStringA(str.str().c_str());

	switch (message)
	{
		case WM_LBUTTONDOWN:
		{
			int xx = GET_X_LPARAM(lParam);
			int yy = GET_Y_LPARAM(lParam);
			POINT p; p.x = xx; p.y = yy;

			if (xx < x || xx >= x + width || yy < y || yy >= y + height)
			{
				::ShowWindow(hWnd, SW_MINIMIZE);
				HWND hOtherAppWindow = ::WindowFromPoint(p);

				if (hOtherAppWindow != hWnd)
				{
					mouse_event(MOUSEEVENTF_LEFTDOWN, p.x, p.y, 0, 0);
					mouse_event(MOUSEEVENTF_LEFTUP, p.x, p.y, 0, 0);
					mouse_event(MOUSEEVENTF_LEFTDOWN, p.x, p.y, 0, 0);
					mouse_event(MOUSEEVENTF_LEFTUP, p.x, p.y, 0, 0);
					return 0;
				}
			}

			return 0;
		}
		/*case WM_SYSCOMMAND:
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
		}*/
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
    /*if (nCode < 0)
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
					
					//std::stringstream str;
					//str << "X = " << x << ", width = " << width << std::endl;
					//::OutputDebugStringA(str.str().c_str());
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
				::SetWindowRgn(hWnd, hRgn, TRUE);
				::MoveWindow(hWnd, x, y, width, height, TRUE);
				::UnhookWindowsHookEx(hMouseHook);
				::SetCursor(hDefaultCursor);
				resizing = false;
				//::Sleep(500);
				::ShowWindow(hWndMask, SW_HIDE);
				::SetActiveWindow(hWnd);
			}
        }
    }*/

    return ::CallNextHookEx(0, nCode, wParam, lParam);
}
