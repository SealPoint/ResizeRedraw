#include "ResizeRedraw.h"

//===================================================================================
LRESULT CALLBACK WindowsProc (HWND hwnd,
                              UINT messageType,
                              WPARAM wParam,
                              LPARAM lParam)
{
	static ResizeRedraw* resizeRedraw = NULL;

    switch (messageType) 
 	{
        case WM_COMMAND:
        {
            return resizeRedraw->onCommand(wParam, lParam);
        }
        case WM_CLOSE:
        {
            return resizeRedraw->onClose(wParam, lParam);
        }
        case WM_CREATE:
        {
			CREATESTRUCTA* createStruct = (CREATESTRUCTA*)lParam;
			resizeRedraw = (ResizeRedraw*)createStruct->lpCreateParams;
            return resizeRedraw->onCreate(wParam, lParam);
        }
		case WM_LBUTTONDOWN:
		{
			return resizeRedraw->onMouseDown(wParam, lParam);
		}
		case WM_LBUTTONUP:
		{
			return resizeRedraw->onMouseUp(wParam, lParam);
		}
		case WM_MOUSEMOVE:
		{
			return resizeRedraw->onMouseMove(wParam, lParam);
		}
        case WM_PAINT:
        {
            return resizeRedraw->onPaint(wParam, lParam);
        }
        default: 
        {
            return ::DefWindowProc(hwnd, messageType, wParam, lParam); 
        }
 	} 

 	return 0;
}

HBRUSH ResizeRedraw::m_backgroundBrush = 0;

//===================================================================================
ResizeRedraw::ResizeRedraw (HINSTANCE hInst,
						    int x,
						    int y,
						    int width,
						    int height)
    : m_hInst(hInst)
    , m_hwnd(0)
    , m_x(x)
    , m_y(y)
    , m_width(width)
    , m_height(height)
	, m_hdcMemory(0)
	, m_prevMouseX(-1)
	, m_prevMouseY(-1)
{
	m_x = 200;
	m_y = 200;
	m_width = 200;
	m_height = 200;
    initialize();
}

//===================================================================================
ResizeRedraw::~ResizeRedraw ()
{
	if (m_hdcMemory)
	{
		::DeleteDC(m_hdcMemory);
	}
}

//===================================================================================
void ResizeRedraw::initialize ()
{   
    WNDCLASSA rwc = {0};
    
    rwc.lpszClassName = RESIZE_REDRAW_CLASS;
    rwc.hbrBackground = (HBRUSH)::GetStockObject(NULL_BRUSH); // transparent
    rwc.lpfnWndProc = WindowsProc;
	rwc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
    rwc.style = CS_HREDRAW | CS_VREDRAW;

    ::RegisterClassA(&rwc); // If fails, it doesn't matter.

    createBrushes();

	HDC hdcScreen = ::GetDC(NULL);

	RECT screenSize;
	::SystemParametersInfoA(SPI_GETWORKAREA, 0, &screenSize, 0);
	m_screenWidth = screenSize.right - screenSize.left;
	m_screenHeight = screenSize.bottom - screenSize.top;
	HBITMAP hDesktopBitmap = ::CreateCompatibleBitmap(hdcScreen, m_screenWidth, m_screenHeight);
	m_hdcMemory = ::CreateCompatibleDC(hdcScreen);
    ::SelectObject(m_hdcMemory, hDesktopBitmap);

    ::BitBlt(m_hdcMemory, // Destination context
             0,
             0, 
             m_screenWidth,
             m_screenHeight,
             hdcScreen, // Source context
             0, // Source x to start from
             0, // Source y to start from
             SRCCOPY); // Copy

	m_hwnd = ::CreateWindowExA(WS_EX_TOPMOST,
							   RESIZE_REDRAW_CLASS,
							   0, // no title
							   WS_POPUP,
							   0,
							   0,
							   m_screenWidth,
							   m_screenHeight,
							   NULL, // no parent window
							   NULL, // no menu
							   m_hInst,
							   this);
}

//===================================================================================
void ResizeRedraw::createBrushes ()
{
	m_backgroundBrush = createBrush(WINDOW_BACKGROUND_COLORREF);
}

//===================================================================================
LRESULT ResizeRedraw::onCommand (WPARAM wParam, LPARAM lParam)
{
    return 0; 
}

//===================================================================================
LRESULT ResizeRedraw::onClose (WPARAM wParam, LPARAM lParam)
{
    return ::DefWindowProc(m_hwnd, WM_CLOSE, wParam, lParam); 
}

//===================================================================================
LRESULT ResizeRedraw::onCreate (WPARAM wParam, LPARAM lParam)
{
    return ::DefWindowProc(m_hwnd, WM_CREATE, wParam, lParam); 
}

//===================================================================================
LRESULT ResizeRedraw::onMouseDown (WPARAM wParam, LPARAM lParam)
{
	int x = GET_X_LPARAM(lParam); 

	if (x >= m_x && x < m_x + m_width)
	{
		int y = GET_Y_LPARAM(lParam);

		if (y >= m_y && y < m_y + m_height)
		{
			m_prevMouseX = x;
			m_prevMouseY = y;
		}
	} 

    return 0; 
}

//===================================================================================
LRESULT ResizeRedraw::onMouseUp (WPARAM wParam, LPARAM lParam)
{
	m_prevMouseX = -1;
	m_prevMouseY = -1; 

    return 0; 
}

//===================================================================================
LRESULT ResizeRedraw::onMouseMove (WPARAM wParam, LPARAM lParam)
{
	if (wParam & MK_LBUTTON && m_prevMouseX >= 0)
	{
		int x = GET_X_LPARAM(lParam); 
		int y = GET_Y_LPARAM(lParam);
		int dx = x - m_prevMouseX;
		int dy = y - m_prevMouseY;

		RECT rect;
		rect.left = min(m_x, m_x + dx);
		rect.right = max(m_x, m_x + dx) + m_width;
		rect.top = min(m_y, m_y + dy);
		rect.bottom = max(m_y, m_y + dy) + m_height;
		m_x += dx;
		m_y += dy;
		::InvalidateRect(m_hwnd, &rect, FALSE);
		m_prevMouseX = x;
		m_prevMouseY = y;
	} 

    return 0; 
}

//===================================================================================
void ResizeRedraw::onMove (int dx, int dy)
{
    RECT rect;
	rect.left = min(m_x, m_x + dx);
	rect.right = max(m_x, m_x + dx) + m_width;
	rect.top = min(m_y, m_y + dy);
	rect.bottom = max(m_y, m_y + dy) + m_height;
	::InvalidateRect(m_hwnd, &rect, FALSE);
}

//===================================================================================
LRESULT ResizeRedraw::onEraseBackground (WPARAM wParam, LPARAM lParam)
{
	return 1;
    //return ::DefWindowProc(m_hwnd, WM_ERASEBKGND, wParam, lParam); 
}

//===================================================================================
LRESULT ResizeRedraw::onPaint (WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
    HDC hdc = ::BeginPaint(m_hwnd, &ps);
	static HDC hdcMem = ::CreateCompatibleDC(hdc);
	static HBITMAP hBitmap = ::CreateCompatibleBitmap(hdc, m_screenWidth, m_screenHeight);
	::SelectObject(hdcMem, hBitmap);

    ::BitBlt(hdcMem, // Destination context
             0,
             0, 
             m_screenWidth,
             m_screenHeight,
             m_hdcMemory, // Source context
             0, // Source x to start from
             0, // Source y to start from
             SRCCOPY); // Copy
	RECT rect;
	rect.left = m_x;
	rect.right = rect.left + m_width;
	rect.top = m_y;
	rect.bottom = rect.top + m_height;
	::FillRect(hdcMem, &rect, m_backgroundBrush);

	::BitBlt(hdc, // Destination context
             0,
             0, 
             m_screenWidth,
             m_screenHeight,
             hdcMem, // Source context
             0, // Source x to start from
             0, // Source y to start from
             SRCCOPY); // Copy
	//::DeleteDC(hdcMem);
    
    ::EndPaint(m_hwnd, &ps);
    ::ReleaseDC(m_hwnd, hdc);
    return 0;
}

//===================================================================================
HPEN ResizeRedraw::createPen (COLORREF rgb, int width)
{
    HPEN hPen = ::CreatePen(PS_SOLID,
                            width,
                            rgb);

    return hPen;
}

//===================================================================================
HBRUSH ResizeRedraw::createBrush (COLORREF rgb)
{
    HBRUSH hBrush = ::CreateSolidBrush(rgb);

    return hBrush;
}

//===================================================================================
void ResizeRedraw::drawBitmap (HDC hdc,
							   int x,
							   int y,
							   HBITMAP hBitmap,
							   int srcX,
							   int srcY,
							   int width,
							   int height)
{
    HDC hdcMem = ::CreateCompatibleDC(hdc);
    ::SelectObject(hdcMem, hBitmap);

    ::BitBlt(hdc, // Destination context
             x,
             y, 
             width,
             height,
             hdcMem, // Source context
             srcX, // Source x to start from
             srcY, // Source y to start from
             SRCCOPY); // Copy
    ::DeleteDC(hdcMem);
}

//===================================================================================
void ResizeRedraw::show ()
{
	::ShowWindow(m_hwnd, SW_SHOW);
	::UpdateWindow(m_hwnd);
}

int APIENTRY WinMain (HINSTANCE hInstance,
                      HINSTANCE hPrevInstance, 
                      LPSTR lpCmdLine, 
                      int nCmdShow)
{
    MSG msg;

    lpCmdLine;
    hPrevInstance;

    ResizeRedraw resizeRedraw(hInstance, 200, 200, 200, 200);
	resizeRedraw.show();

    // Acquire and dispatch messages until a WM_QUIT message is received.
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);  // Translates virtual key codes
        DispatchMessage(&msg);   // Dispatches message to window
    }

    return (int)msg.wParam;  // Returns the value from PostQuitMessage
}
