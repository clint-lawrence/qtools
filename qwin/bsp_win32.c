/*****************************************************************************
* Product: BSP for QWIN GUI demo
* Last updated for version: 6.9.3
* Date of the Last Update:  2021-03-03
*
*                    Q u a n t u m  L e a P s
*                    ------------------------
*                    Modern Embedded Software
*
* Copyright (C) 2005-2021 Quantum Leaps, LLC. All rights reserved.
*
* This program is open source software: you can redistribute it and/or
* modify it under the terms of the following MIT License (MIT).
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
*
* Contact information:
* <www.state-machine.com/licensing>
* <info@state-machine.com>
*****************************************************************************/
#include <stdint.h>

#include "bsp.h"  /* BSP interface */

#include "qwin_gui.h"  /* QWIN GUI */
#include "resource.h"  /* GUI resource IDs generated by resource editior */

/* local variables ---------------------------------------------------------*/
static HINSTANCE l_hInst;   /* this application instance */
static HWND      l_hWnd;    /* main window handle */
static LPSTR     l_cmdLine; /* the command line string */

static GraphicDisplay   l_oled; /* the OLED display of the EK-LM3S811 board */
static SegmentDisplay   l_userLED;    /* USER LED of the EK-LM3S811 board */
static SegmentDisplay   l_scoreBoard; /* segment display for the score */
static OwnerDrawnButton l_userBtn;   /* USER button of the EK-LM3S811 board */

/* (R,G,B) colors for the OLED display */
static BYTE const c_onColor [3] = { 255U, 255U,   0U }; /* yellow */
static BYTE const c_offColor[3] = {  15U,  15U,  15U }; /* very dark gray */

static int l_paused;

/* Local functions ---------------------------------------------------------*/
static LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg,
                                WPARAM wParam, LPARAM lParam);

/*..........................................................................*/
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst,
                   LPSTR cmdLine, int iCmdShow)
{
    HWND hWnd;
    MSG  msg;

    (void)hPrevInst; /* unused parameter */

    l_hInst   = hInst;   /* save the application instance */
    l_cmdLine = cmdLine; /* save the command line string */

    /* create the main custom dialog window */
    hWnd = CreateCustDialog(hInst, IDD_APPLICATION, NULL,
                            &WndProc, "MY_CLASS");
    ShowWindow(hWnd, iCmdShow); /* show the main window */

    /* enter the message loop... */
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}
/*..........................................................................*/
/* thread function for running the application main_gui() */
static DWORD WINAPI appThread(LPVOID par) {
    (void)par;         /* unused parameter */
    return main_gui(); /* run the application */
}
/*..........................................................................*/
static LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg,
                                WPARAM wParam, LPARAM lParam)
{
    switch (iMsg) {

        /* Perform initialization upon cration of the main dialog window
        * NOTE: Any child-windows are NOT created yet at this time, so
        * the GetDlgItem() function can't be used (it will return NULL).
        */
        case WM_CREATE: {
            l_hWnd = hWnd; /* save the window handle */

            /* initialize the owner-drawn buttons...
            * NOTE: must be done *before* the first drawing of the buttons,
            * so WM_INITDIALOG is too late.
            */
            OwnerDrawnButton_init(&l_userBtn, IDC_USER,
                       LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_BTN_UP)),
                       LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_BTN_DWN)),
                       LoadCursor(NULL, IDC_HAND));
            return 0;
        }

        /* Perform initialization after all child windows have been created */
        case WM_INITDIALOG: {
            GraphicDisplay_init(&l_oled,
                       BSP_SCREEN_WIDTH,  BSP_SCREEN_HEIGHT,
                       IDC_LCD, c_offColor);

            SegmentDisplay_init(&l_userLED,
                                1U,   /* 1 "segment" (the LED itself) */
                                2U);  /* 2 bitmaps (for LED OFF/ON states) */
            SegmentDisplay_initSegment(&l_userLED, 0U, IDC_LED);
            SegmentDisplay_initBitmap(&l_userLED,
                0U, LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_LED_OFF)));
            SegmentDisplay_initBitmap(&l_userLED,
                 1U, LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_LED_ON)));

            SegmentDisplay_init(&l_scoreBoard,
                                4U,   /* 4 "segments" (digits 0-3) */
                                10U); /* 10 bitmaps (for 0-9 states) */
            SegmentDisplay_initSegment(&l_scoreBoard, 0U, IDC_SEG0);
            SegmentDisplay_initSegment(&l_scoreBoard, 1U, IDC_SEG1);
            SegmentDisplay_initSegment(&l_scoreBoard, 2U, IDC_SEG2);
            SegmentDisplay_initSegment(&l_scoreBoard, 3U, IDC_SEG3);
            SegmentDisplay_initBitmap(&l_scoreBoard,
                 0U, LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_SEG0)));
            SegmentDisplay_initBitmap(&l_scoreBoard,
                 1U, LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_SEG1)));
            SegmentDisplay_initBitmap(&l_scoreBoard,
                 2U, LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_SEG2)));
            SegmentDisplay_initBitmap(&l_scoreBoard,
                 3U, LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_SEG3)));
            SegmentDisplay_initBitmap(&l_scoreBoard,
                 4U, LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_SEG4)));
            SegmentDisplay_initBitmap(&l_scoreBoard,
                 5U, LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_SEG5)));
            SegmentDisplay_initBitmap(&l_scoreBoard,
                 6U, LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_SEG6)));
            SegmentDisplay_initBitmap(&l_scoreBoard,
                 7U, LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_SEG7)));
            SegmentDisplay_initBitmap(&l_scoreBoard,
                 8U, LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_SEG8)));
            SegmentDisplay_initBitmap(&l_scoreBoard,
                 9U, LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_SEG9)));

            BSP_setPaused(0);

            /* --> Spawn the application thread to run main_gui() */
            CreateThread(NULL, 0, &appThread, NULL, 0, NULL);

            return 0;
        }

        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }

        /* commands from child controls and menus... */
        case WM_COMMAND: {
            switch (wParam) {
                case IDOK:
                case IDCANCEL: {
                    PostQuitMessage(0);
                    break;
                }
                case IDC_USER: { /* owner-drawn button(s) */
                    SetFocus(hWnd);
                    break;
                }
            }
            return 0;
        }

        /* drawing of owner-drawn buttons... */
        case WM_DRAWITEM: {
            LPDRAWITEMSTRUCT pdis = (LPDRAWITEMSTRUCT)lParam;
            switch (pdis->CtlID) {
                case IDC_USER: { /* USER owner-drawn button */
                    switch (OwnerDrawnButton_draw(&l_userBtn, pdis)) {
                        case BTN_DEPRESSED: {
                            BSP_setPaused(1);
                            SegmentDisplay_setSegment(&l_userLED, 0U, 1U);
                            break;
                        }
                        case BTN_RELEASED: {
                            BSP_setPaused(0);
                            SegmentDisplay_setSegment(&l_userLED, 0U, 0U);
                            break;
                        }
                        default: {
                            break;
                        }
                    }
                    break;
                }
            }
            return 0;
        }

        /* mouse input... */
        case WM_MOUSEWHEEL: {
            /* wheel turned forward? */
            if ((HIWORD(wParam) & 0x8000U) == 0U) {
                BSP_setPaused(1);
            }
            else { /* the wheel was turned backwards */
                BSP_setPaused(0);
            }
            return 0;
        }

        /* keyboard input... */
        case WM_KEYDOWN: {
            switch (wParam) {
                case VK_SPACE:
                    BSP_setPaused(!l_paused);
                    break;
            }
            return 0;
        }

    }
    return DefWindowProc(hWnd, iMsg, wParam, lParam) ;
}
/*..........................................................................*/
void BSP_init(void) {
}
/*..........................................................................*/
void BSP_terminate(int result) {
    PostQuitMessage(result); /* post the Quit message to the GUI */
}
/*..........................................................................*/
void BSP_sleep(uint32_t ticks) {
    Sleep(1000U * ticks  / BSP_TICKS_PER_SEC);
}
/*..........................................................................*/
void BSP_setPaused(int paused) {
    l_paused = paused;
    if (l_paused) {
        GraphicDisplay_clear(&l_oled);
        BSP_drawNString(35U, 0U, "PAUSED");
        GraphicDisplay_redraw(&l_oled);

        SetDlgItemText(l_hWnd, IDC_PAUSED, "PAUSED");
    }
    else {
        SetDlgItemText(l_hWnd, IDC_PAUSED, "RUNNING");
    }
}
/*..........................................................................*/
int BSP_isPaused(void) {
    return l_paused;
}

/*..........................................................................*/
void BSP_drawBitmap(uint8_t const *bitmap) {
    UINT x, y;
    /* map the EK-LM3S811 OLED pixels to the GraphicDisplay pixels... */
    for (y = 0; y < BSP_SCREEN_HEIGHT; ++y) {
        for (x = 0; x < BSP_SCREEN_WIDTH; ++x) {
            uint8_t bits = bitmap[x + (y/8U)*BSP_SCREEN_WIDTH];
            if ((bits & (1U << (y & 0x07U))) != 0U) {
                GraphicDisplay_setPixel(&l_oled, x, y, c_onColor);
            }
            else {
                GraphicDisplay_clearPixel(&l_oled, x, y);
            }
        }
    }

    /* draw the updated GraphicDisplay on the screen */
    GraphicDisplay_redraw(&l_oled);
}
/*..........................................................................*/
void BSP_drawCount(uint32_t n) {
    /* update the score in the l_scoreBoard SegmentDisplay */
    SegmentDisplay_setSegment(&l_scoreBoard, 0U, (UINT)(n % 10U));
    n /= 10U;
    SegmentDisplay_setSegment(&l_scoreBoard, 1U, (UINT)(n % 10U));
    n /= 10U;
    SegmentDisplay_setSegment(&l_scoreBoard, 2U, (UINT)(n % 10U));
    n /= 10U;
    SegmentDisplay_setSegment(&l_scoreBoard, 3U, (UINT)(n % 10U));
}
/*..........................................................................*/
void BSP_drawNString(uint8_t x, uint8_t y, char const *str) {
    static uint8_t const font5x7[95][5] = {
        { 0x00, 0x00, 0x00, 0x00, 0x00 },  /* ' ' */
        { 0x00, 0x00, 0x4F, 0x00, 0x00 },  /* ! */
        { 0x00, 0x07, 0x00, 0x07, 0x00 },  /* " */
        { 0x14, 0x7F, 0x14, 0x7F, 0x14 },  /* # */
        { 0x24, 0x2A, 0x7F, 0x2A, 0x12 },  /* $ */
        { 0x23, 0x13, 0x08, 0x64, 0x62 },  /* % */
        { 0x36, 0x49, 0x55, 0x22, 0x50 },  /* & */
        { 0x00, 0x05, 0x03, 0x00, 0x00 },  /* ' */
        { 0x00, 0x1C, 0x22, 0x41, 0x00 },  /* ( */
        { 0x00, 0x41, 0x22, 0x1C, 0x00 },  /* ) */
        { 0x14, 0x08, 0x3E, 0x08, 0x14 },  /* * */
        { 0x08, 0x08, 0x3E, 0x08, 0x08 },  /* + */
        { 0x00, 0x50, 0x30, 0x00, 0x00 },  /* , */
        { 0x08, 0x08, 0x08, 0x08, 0x08 },  /* - */
        { 0x00, 0x60, 0x60, 0x00, 0x00 },  /* . */
        { 0x20, 0x10, 0x08, 0x04, 0x02 },  /* / */
        { 0x3E, 0x51, 0x49, 0x45, 0x3E },  /* 0 */
        { 0x00, 0x42, 0x7F, 0x40, 0x00 },  /* 1 */
        { 0x42, 0x61, 0x51, 0x49, 0x46 },  /* 2 */
        { 0x21, 0x41, 0x45, 0x4B, 0x31 },  /* 3 */
        { 0x18, 0x14, 0x12, 0x7F, 0x10 },  /* 4 */
        { 0x27, 0x45, 0x45, 0x45, 0x39 },  /* 5 */
        { 0x3C, 0x4A, 0x49, 0x49, 0x30 },  /* 6 */
        { 0x01, 0x71, 0x09, 0x05, 0x03 },  /* 7 */
        { 0x36, 0x49, 0x49, 0x49, 0x36 },  /* 8 */
        { 0x06, 0x49, 0x49, 0x29, 0x1E },  /* 9 */
        { 0x00, 0x36, 0x36, 0x00, 0x00 },  /* : */
        { 0x00, 0x56, 0x36, 0x00, 0x00 },  /* ; */
        { 0x08, 0x14, 0x22, 0x41, 0x00 },  /* < */
        { 0x14, 0x14, 0x14, 0x14, 0x14 },  /* = */
        { 0x00, 0x41, 0x22, 0x14, 0x08 },  /* > */
        { 0x02, 0x01, 0x51, 0x09, 0x06 },  /* ? */
        { 0x32, 0x49, 0x79, 0x41, 0x3E },  /* @ */
        { 0x7E, 0x11, 0x11, 0x11, 0x7E },  /* A */
        { 0x7F, 0x49, 0x49, 0x49, 0x36 },  /* B */
        { 0x3E, 0x41, 0x41, 0x41, 0x22 },  /* C */
        { 0x7F, 0x41, 0x41, 0x22, 0x1C },  /* D */
        { 0x7F, 0x49, 0x49, 0x49, 0x41 },  /* E */
        { 0x7F, 0x09, 0x09, 0x09, 0x01 },  /* F */
        { 0x3E, 0x41, 0x49, 0x49, 0x7A },  /* G */
        { 0x7F, 0x08, 0x08, 0x08, 0x7F },  /* H */
        { 0x00, 0x41, 0x7F, 0x41, 0x00 },  /* I */
        { 0x20, 0x40, 0x41, 0x3F, 0x01 },  /* J */
        { 0x7F, 0x08, 0x14, 0x22, 0x41 },  /* K */
        { 0x7F, 0x40, 0x40, 0x40, 0x40 },  /* L */
        { 0x7F, 0x02, 0x0C, 0x02, 0x7F },  /* M */
        { 0x7F, 0x04, 0x08, 0x10, 0x7F },  /* N */
        { 0x3E, 0x41, 0x41, 0x41, 0x3E },  /* O */
        { 0x7F, 0x09, 0x09, 0x09, 0x06 },  /* P */
        { 0x3E, 0x41, 0x51, 0x21, 0x5E },  /* Q */
        { 0x7F, 0x09, 0x19, 0x29, 0x46 },  /* R */
        { 0x46, 0x49, 0x49, 0x49, 0x31 },  /* S */
        { 0x01, 0x01, 0x7F, 0x01, 0x01 },  /* T */
        { 0x3F, 0x40, 0x40, 0x40, 0x3F },  /* U */
        { 0x1F, 0x20, 0x40, 0x20, 0x1F },  /* V */
        { 0x3F, 0x40, 0x38, 0x40, 0x3F },  /* W */
        { 0x63, 0x14, 0x08, 0x14, 0x63 },  /* X */
        { 0x07, 0x08, 0x70, 0x08, 0x07 },  /* Y */
        { 0x61, 0x51, 0x49, 0x45, 0x43 },  /* Z */
        { 0x00, 0x7F, 0x41, 0x41, 0x00 },  /* [ */
        { 0x02, 0x04, 0x08, 0x10, 0x20 },  /* \ */
        { 0x00, 0x41, 0x41, 0x7F, 0x00 },  /* ] */
        { 0x04, 0x02, 0x01, 0x02, 0x04 },  /* ^ */
        { 0x40, 0x40, 0x40, 0x40, 0x40 },  /* _ */
        { 0x00, 0x01, 0x02, 0x04, 0x00 },  /* ` */
        { 0x20, 0x54, 0x54, 0x54, 0x78 },  /* a */
        { 0x7F, 0x48, 0x44, 0x44, 0x38 },  /* b */
        { 0x38, 0x44, 0x44, 0x44, 0x20 },  /* c */
        { 0x38, 0x44, 0x44, 0x48, 0x7F },  /* d */
        { 0x38, 0x54, 0x54, 0x54, 0x18 },  /* e */
        { 0x08, 0x7E, 0x09, 0x01, 0x02 },  /* f */
        { 0x0C, 0x52, 0x52, 0x52, 0x3E },  /* g */
        { 0x7F, 0x08, 0x04, 0x04, 0x78 },  /* h */
        { 0x00, 0x44, 0x7D, 0x40, 0x00 },  /* i */
        { 0x20, 0x40, 0x44, 0x3D, 0x00 },  /* j */
        { 0x7F, 0x10, 0x28, 0x44, 0x00 },  /* k */
        { 0x00, 0x41, 0x7F, 0x40, 0x00 },  /* l */
        { 0x7C, 0x04, 0x18, 0x04, 0x78 },  /* m */
        { 0x7C, 0x08, 0x04, 0x04, 0x78 },  /* n */
        { 0x38, 0x44, 0x44, 0x44, 0x38 },  /* o */
        { 0x7C, 0x14, 0x14, 0x14, 0x08 },  /* p */
        { 0x08, 0x14, 0x14, 0x18, 0x7C },  /* q */
        { 0x7C, 0x08, 0x04, 0x04, 0x08 },  /* r */
        { 0x48, 0x54, 0x54, 0x54, 0x20 },  /* s */
        { 0x04, 0x3F, 0x44, 0x40, 0x20 },  /* t */
        { 0x3C, 0x40, 0x40, 0x20, 0x7C },  /* u */
        { 0x1C, 0x20, 0x40, 0x20, 0x1C },  /* v */
        { 0x3C, 0x40, 0x30, 0x40, 0x3C },  /* w */
        { 0x44, 0x28, 0x10, 0x28, 0x44 },  /* x */
        { 0x0C, 0x50, 0x50, 0x50, 0x3C },  /* y */
        { 0x44, 0x64, 0x54, 0x4C, 0x44 },  /* z */
        { 0x00, 0x08, 0x36, 0x41, 0x00 },  /* { */
        { 0x00, 0x00, 0x7F, 0x00, 0x00 },  /* | */
        { 0x00, 0x41, 0x36, 0x08, 0x00 },  /* } */
        { 0x02, 0x01, 0x02, 0x04, 0x02 },  /* ~ */
    };
    UINT dx, dy;

    while (*str != '\0') {
        uint8_t const *ch = &font5x7[*str - ' '][0];
        for (dx = 0U; dx < 5U; ++dx) {
            for (dy = 0U; dy < 8U; ++dy) {
                if ((ch[dx] & (1U << dy)) != 0U) {
                    GraphicDisplay_setPixel(&l_oled, (UINT)(x + dx),
                                         (UINT)(y*8U + dy), c_onColor);
                }
                else {
                    GraphicDisplay_clearPixel(&l_oled, (UINT)(x + dx),
                                           (UINT)(y*8U + dy));
                }
            }
        }
        ++str;
        x += 6;
    }
    /* draw the updated GraphicDisplay on the screen */
    GraphicDisplay_redraw(&l_oled);
}
