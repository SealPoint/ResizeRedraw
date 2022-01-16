#pragma once

#include "resource.h"
#define SC_SZLEFT 0xF001

#ifndef RESIZE_REDRAW_H
#define RESIZE_REDRAW_H 1

#include <windows.h>
#include <windowsx.h>

#define RESIZE_REDRAW_CLASS "ResizeRedraw"

#define OUTER_CORNER_RADIUS 8

#define COMPONENT_BORDER_COLORREF RGB(115, 130, 222)
#define COMPONENT_BACKGROUND_COLORREF RGB(255, 255, 255)
#define WINDOW_BACKGROUND_COLORREF RGB(200, 191, 231)
#define WINDOW_BORDER_COLORREF RGB(64, 0, 128)
#define WINDOW_BORDER_WIDTH 6
#define WINDOW_INNER_BORDER_COLORREF RGB(132, 96, 178)
#define WINDOW_CORNER_RADIUS 8
#define COMPONENT_TEXT_COLORREF RGB(0, 0, 0)

/// This is a Windows callback function that is called
/// each type a message is sent to a window.
/// @param hwnd The handle to the window
/// @param messageType The type of the message
/// @param wParam A message-specific parameter as DWORD
/// @param lParam A message-specific parameter as DWORD
LRESULT CALLBACK WindowsProc (HWND hwnd,
                              UINT messageType,
                              WPARAM wParam,
                              LPARAM lParam);

/// @header "" "ResizeRedraw.h"
/// This is a window that occupies the entire screen
/// and the actual "Window" is a painted region within the screen
class ResizeRedraw
{
public:
    
	ResizeRedraw (HINSTANCE hInst,
                  int x,
                  int y,
                  int width,
                  int height);
    virtual ~ResizeRedraw ();

    /// Creates an RGB bitmap object from a resource
    /// @param hInstance The instance of the process
    /// @param resourceId The resource ID of the bitmap
    /// @param createDibSection If true, the RGB buffer data for
    ///    the bitmap is loaded
    /// @return The bitmap handle
    static HBITMAP createBitmap (HINSTANCE hInstance,
                                 int resourceId,
                                 bool createDibSection);

    /// Creates a pen object
    /// @param rgb The RGB value of the pen
    /// @return The pen handle
    static HPEN createPen (COLORREF rgb, int width = 1);

    /// Creates a brush object
    /// @param rgb The RGB value of the brush
    /// @return The brush handle
    static HBRUSH createBrush (COLORREF rgb);

    /// Create the necessary brushes - default implementation
    void createBrushes ();

    /// Event handlers - default implementations
    LRESULT onCommand (WPARAM wParam, LPARAM lParam);
    LRESULT onClose (WPARAM wParam, LPARAM lParam);
    LRESULT onCreate (WPARAM wParam, LPARAM lParam);
	LRESULT onMouseMove (WPARAM wParam, LPARAM lParam);
	LRESULT onMouseDown (WPARAM wParam, LPARAM lParam);
	LRESULT onMouseUp (WPARAM wParam, LPARAM lParam);
    void onMove (int dx, int dy);
    LRESULT onEraseBackground (WPARAM wParam, LPARAM lParam);
    LRESULT onPaint (WPARAM wParam, LPARAM lParam);

    /// Draws a bitmap
    static void drawBitmap (HDC hdc,
							int x,
							int y,
							HBITMAP hBitmap,
							int srcX = 0,
							int srcY = 0,
							int width = -1,
							int height = -1);

	void show ();

private:

	void initialize ();

    /// Registers a Windows class for a specific window.
    /// @param wndClassName The name of the window class to register
    /// @param backgroundRGB The initial background RGB
    /// @param isTransparent If true, this component is transparent
    /// @param isDoubleClick If true double click is enabled
    /// @param hCursor The handle to the cursor
    /// @return TRUE if the class was registered, false otherwise
    static BOOL registerWindowClass (const char* wndClassName,
                                     const COLORREF& colorRef);

	int m_screenWidth;
	int m_screenHeight;

    /// The process instance
    HINSTANCE m_hInst;

    /// The handle of this component, since every component
    /// is a window.
    HWND m_hwnd;

    /// x-coordinate on the parent component
    int m_x;

    /// y-coordinate on the parent component
    int m_y;

    /// Width
    int m_width;

    /// Height
    int m_height;

	/// The background brush
    static HBRUSH m_backgroundBrush;

	/// Memory context holding the desktop bitmap
	HDC m_hdcMemory;

	int m_prevMouseX;
	int m_prevMouseY;
};

#endif
