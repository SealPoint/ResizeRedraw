#ifndef RR_CAPTION_H
#define RR_CAPTION_H 1

#include <windows.h>

#define CAPTION_HEIGHT 50
#define CAPTION_LEFT_PAD 10
#define CAPTION_TOP_PAD 5
#define CAPTION_COLORREF RGB(0, 0x47, 0xAB)

void InitCaption (int startX, int width);
void DrawCaption (HDC hdc, HBITMAP hBitmap);
#endif