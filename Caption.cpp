#include "Caption.h"

/// The data about the gradient
TRIVERTEX leftGradientVertices[2];
TRIVERTEX rightGradientVertices[2];

/// The data about the gradient
TRIVERTEX leftGradientDisabledVertices[2];
TRIVERTEX rightGradientDisabledVertices[2];

int captionRed = 0, captionGreen = 0x4700, captionBlue = 0xAB00;
int gradientRed = 0xD200, gradientGreen = 0xE600, gradientBlue = 0xFF00;

//===================================================================================
void InitCaption (int startX, int width)
{
	leftGradientVertices[0].x = startX;
    leftGradientVertices[0].y = 0;
    leftGradientVertices[0].Red = captionRed;
    leftGradientVertices[0].Green = captionGreen;
    leftGradientVertices[0].Blue = captionBlue;
    leftGradientVertices[0].Alpha = 0x0000;

    leftGradientVertices[1].x = startX + width / 2;
    leftGradientVertices[1].y = CAPTION_HEIGHT;
    leftGradientVertices[1].Red = gradientRed;
    leftGradientVertices[1].Green = gradientGreen;
    leftGradientVertices[1].Blue = gradientBlue;
    leftGradientVertices[1].Alpha = 0x0000;

    rightGradientVertices[0].x = startX + width - 1;
    rightGradientVertices[0].y = CAPTION_HEIGHT;
    rightGradientVertices[0].Red = captionRed;
    rightGradientVertices[0].Green = captionGreen;
    rightGradientVertices[0].Blue = captionBlue;
    rightGradientVertices[0].Alpha = 0x0000;

    rightGradientVertices[1].x = startX + width / 2;
    rightGradientVertices[1].y = 0;
    rightGradientVertices[1].Red = gradientRed;
    rightGradientVertices[1].Green = gradientGreen;
    rightGradientVertices[1].Blue = gradientBlue;
    rightGradientVertices[1].Alpha = 0x0000;
}

//===================================================================================
void DrawCaption (HDC hdc, HBITMAP hBitmap)
{
	GRADIENT_RECT gradRect;
    gradRect.UpperLeft = 0;
    gradRect.LowerRight = 1;
    ::GradientFill(hdc, leftGradientVertices, 2, &gradRect, 1, GRADIENT_FILL_RECT_H);

    gradRect.UpperLeft = 1;
    gradRect.LowerRight = 0;
    ::GradientFill(hdc, rightGradientVertices, 2, &gradRect, 1, GRADIENT_FILL_RECT_H);

	if (hBitmap)
	{
		BITMAP bitmap;
		memset(&bitmap, 0, sizeof(BITMAP));
		::GetObject(hBitmap, sizeof(BITMAP), &bitmap);

        HDC hdcMem = ::CreateCompatibleDC(hdc);
        ::SelectObject(hdcMem, hBitmap);

        ::BitBlt(hdc, // Destination context
                 leftGradientVertices[0].x + CAPTION_LEFT_PAD, // x
                 CAPTION_TOP_PAD, // y
                 bitmap.bmWidth, // Bitmap width
                 bitmap.bmHeight, // Bitmap height
                 hdcMem, // Source context
                 0, // Source x to start from
                 0, // Source y to start from
                 SRCCOPY); // Copy
        ::DeleteDC(hdcMem);
	}
}