#include <windows.h>
#include <commctrl.h>
#include <wchar.h>
#include "resource.h"

// Use our own count macro to avoid collision
#ifndef COUNT_OF
#define COUNT_OF(a) (sizeof(a)/sizeof((a)[0]))
#endif

static HWND   hDisplay;
static double operand = 0.0;
static int    lastOp  = 0;    // '+','-','*','/','=' or 0

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
int     WINAPI       wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int);
int     WINAPI       WinMain(HINSTANCE, HINSTANCE, LPSTR,  int);

// ANSI stub for -mwindows; forwards to Unicode
int WINAPI WinMain(
    HINSTANCE hInst,
    HINSTANCE hPrev,
    LPSTR     lpCmdLine,
    int       nShowCmd
) {
    return wWinMain(hInst, hPrev, GetCommandLineW(), nShowCmd);
}

// Unicode entry point
int WINAPI wWinMain(
    HINSTANCE hInst,
    HINSTANCE hPrev,
    LPWSTR    lpCmdLine,
    int       nShowCmd
) {
    INITCOMMONCONTROLSEX ic = { sizeof(ic), ICC_WIN95_CLASSES };
    InitCommonControlsEx(&ic);

    WNDCLASSEXW wc = { sizeof(wc) };
    wc.style          = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc    = WndProc;
    wc.hInstance      = hInst;
    wc.hIcon          = LoadIconW(hInst, MAKEINTRESOURCEW(IDI_APP_ICON));
    wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground  = GetSysColorBrush(COLOR_WINDOW);
    wc.lpszClassName  = L"CalcClass";
    RegisterClassExW(&wc);

    HWND hWnd = CreateWindowExW(
        WS_EX_CLIENTEDGE | WS_EX_COMPOSITED,
        wc.lpszClassName, L"Calculator",
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, 260, 350,
        NULL,
        LoadMenuW(hInst, MAKEINTRESOURCEW(IDR_MAINMENU)),
        hInst, NULL
    );

    ShowWindow(hWnd, nShowCmd);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}

// Append text to the end of the display
void AppendToDisplay(const wchar_t *txt) {
    int len = GetWindowTextLengthW(hDisplay);
    SendMessageW(hDisplay, EM_SETSEL, (WPARAM)len, (LPARAM)len);
    SendMessageW(hDisplay, EM_REPLACESEL, 0, (LPARAM)txt);
}

// Perform pending operation or set new operator
void DoOperation(int op) {
    wchar_t buf[64];
    GetWindowTextW(hDisplay, buf, COUNT_OF(buf));
    double val = wcstod(buf, NULL);

    if (lastOp) {
        switch (lastOp) {
        case '+': operand += val; break;
        case '-': operand -= val; break;
        case '*': operand *= val; break;
        case '/': operand = val != 0.0 ? operand / val : 0.0; break;
        }
    } else {
        operand = val;
    }

    if (op == '=') {
        swprintf(buf, COUNT_OF(buf), L"%g", operand);
        SetWindowTextW(hDisplay, buf);
        lastOp = 0;
    } else {
        lastOp = op;
        SetWindowTextW(hDisplay, L"0");
    }
}

// Main window procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        hDisplay = CreateWindowExW(
            0, L"EDIT", L"0",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_RIGHT | ES_READONLY,
            10, 10, 220, 30, hWnd,
            (HMENU)(INT_PTR)IDC_DISPLAY,
            ((LPCREATESTRUCT)lParam)->hInstance, NULL
        );

        const wchar_t *labels[] = {
            L"7", L"8", L"9", L"/",
            L"4", L"5", L"6", L"*",
            L"1", L"2", L"3", L"-",
            L"C", L"0", L"=", L"+",
        };
        const int ids[] = {
            IDC_BTN_7, IDC_BTN_8, IDC_BTN_9, IDC_BTN_DIV,
            IDC_BTN_4, IDC_BTN_5, IDC_BTN_6, IDC_BTN_MUL,
            IDC_BTN_1, IDC_BTN_2, IDC_BTN_3, IDC_BTN_SUB,
            IDC_BTN_CLR,IDC_BTN_0, IDC_BTN_EQ,  IDC_BTN_ADD,
        };

        for (int i = 0; i < COUNT_OF(labels); i++) {
            int row = i / 4, col = i % 4;
            CreateWindowExW(
                0, L"BUTTON", labels[i],
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                10 + col * 55, 50 + row * 55, 50, 50,
                hWnd, (HMENU)(INT_PTR)ids[i],
                ((LPCREATESTRUCT)lParam)->hInstance, NULL
            );
        }
        break;
    }

    case WM_COMMAND: {
        int id = LOWORD(wParam);
        if      (id >= IDC_BTN_0 && id <= IDC_BTN_9) {
            wchar_t d[2] = { L'0' + (id - IDC_BTN_0), 0 };
            AppendToDisplay(d);
        }
        else if (id == IDC_BTN_CLR) {
            SetWindowTextW(hDisplay, L"0");
            operand = 0.0; lastOp = 0;
        }
        else if (id == IDC_BTN_ADD) DoOperation(L'+');
        else if (id == IDC_BTN_SUB) DoOperation(L'-');
        else if (id == IDC_BTN_MUL) DoOperation(L'*');
        else if (id == IDC_BTN_DIV) DoOperation(L'/');
        else if (id == IDC_BTN_EQ ) DoOperation(L'=');
        else if (id == IDM_EXIT)   PostMessageW(hWnd, WM_CLOSE, 0, 0);
        break;
    }

    case WM_ERASEBKGND:
        // block default erasure to prevent flicker
        return 1;

    case WM_PAINT: {
        // paint background immediately to avoid black flash
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        FillRect(hdc, &ps.rcPaint, GetSysColorBrush(COLOR_WINDOW));
        EndPaint(hWnd, &ps);
        return 0;
    }

    case WM_SYSCOMMAND:
        if ((wParam & 0xFFF0) == SC_MINIMIZE ||
            (wParam & 0xFFF0) == SC_MAXIMIZE ||
            (wParam & 0xFFF0) == SC_RESTORE) {
            return DefWindowProcW(hWnd, msg, wParam, lParam);
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hWnd, msg, wParam, lParam);
}