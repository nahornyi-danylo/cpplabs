#include <windows.h>
#include <string>
#include "resource.h"

static HINSTANCE g_hInst = NULL;

INT_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_INITDIALOG:
        return TRUE;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, 0);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case IDM_EXIT:
            DestroyWindow(hWnd);
            return 0;
        case IDM_ABOUT:
            DialogBoxParamW(g_hInst, MAKEINTRESOURCEW(IDD_ABOUT), hWnd, AboutDlgProc, 0);
            return 0;
        default:
            break;
        }
        break;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        RECT rect;
        GetClientRect(hWnd, &rect);

        HDC hdcScreen = GetDC(NULL);
        int deviceHeightMM = GetDeviceCaps(hdcScreen, VERTSIZE);
        int deviceWidthMM  = GetDeviceCaps(hdcScreen, HORZSIZE);
        ReleaseDC(NULL, hdcScreen);

        std::wstring text =
            L"Висота екрану: " + std::to_wstring(deviceHeightMM) + L" мм\n" +
            L"Ширина екрану: " + std::to_wstring(deviceWidthMM) + L" мм";

        DrawTextW(hdc, text.c_str(), -1, &rect, DT_CENTER | DT_VCENTER | DT_WORDBREAK);

        EndPaint(hWnd, &ps);
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow) {
    g_hInst = hInstance;

    // Register a window class
    WNDCLASSEXW wc = {0};
    wc.cbSize        = sizeof(WNDCLASSEXW);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.hCursor       = LoadCursorW(NULL, IDC_ARROW);
    wc.hIcon         = LoadIconW(NULL, IDI_APPLICATION);
    wc.hIconSm       = wc.hIcon;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);               // system color brush
    wc.lpszClassName = L"Lab1Class";

    if (!RegisterClassExW(&wc)) {
        MessageBoxW(NULL, L"RegisterClassEx failed!", L"Error", MB_ICONERROR);
        return 1;
    }

    // Create the window
    HWND hWnd = CreateWindowW(
        L"Lab1Class", L"Lab1 Window",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 420,
        NULL, NULL, hInstance, NULL
    );
    if (!hWnd) {
        MessageBoxW(NULL, L"CreateWindow failed!", L"Error", MB_ICONERROR);
        return 1;
    }

    // Attach a menu loaded from resources
    HMENU hMenu = LoadMenuW(hInstance, MAKEINTRESOURCEW(IDR_MAINMENU));
    SetMenu(hWnd, hMenu);

    // Load accelerators
    HACCEL hAccel = LoadAcceleratorsW(hInstance, MAKEINTRESOURCEW(IDR_ACCEL));

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // Message loop that honors accelerators
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        if (!TranslateAcceleratorW(hWnd, hAccel, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
    return (int)msg.wParam;
}
