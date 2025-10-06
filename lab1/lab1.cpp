#include <windows.h>
#include <string>

static HINSTANCE hInst;

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_PAINT: 
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            RECT rect;
            GetClientRect(hWnd, &rect);

            int cursorHeight = GetSystemMetrics(SM_CYCURSOR);
            int cursorWidth = GetSystemMetrics(SM_CXCURSOR);
            int clientHeight = rect.bottom - rect.top;
            int clientWidth = rect.right - rect.left;


            std::wstring text =
                L"Висота курсору: " + std::to_wstring(cursorHeight) + L"\n" +
                L"Ширина курсору: " + std::to_wstring(cursorWidth) + L"\n" +
                L"Висота клієнтської області додатка: " + std::to_wstring(clientHeight) + L"\n" +
                L"Ширина клієнтської області додатка: " + std::to_wstring(clientWidth);

            DrawTextW(hdc, text.c_str(), -1, &rect, DT_CENTER | DT_VCENTER | DT_WORDBREAK);

            EndPaint(hWnd, &ps);
            return 0;
        }
        
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProcW(hWnd, msg, wParam, lParam);
    }
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow) {
    hInst = hInstance;

    WNDCLASSEXW wc = {0};
    wc.cbSize        = sizeof(WNDCLASSEXW);
    wc.style         = CS_NOCLOSE | CS_VREDRAW | CS_HREDRAW;
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.hCursor       = LoadCursorW(NULL, IDC_SIZE);
    wc.hIcon         = LoadIconW(NULL, IDI_WINLOGO);
    wc.hbrBackground = (HBRUSH)GetStockObject(DKGRAY_BRUSH);               
    wc.lpszClassName = L"Nahornyi Danylo + Kolyada Maxim";

    if (!RegisterClassExW(&wc)) {
        MessageBoxW(NULL, L"RegisterClassEx failed!", L"Error", MB_ICONERROR);
        return 1;
    }

    HWND hWnd = CreateWindowW(
        L"Nahornyi Danylo + Kolyada Maxim", L"Lab1",
        WS_HSCROLL | WS_OVERLAPPEDWINDOW,
        50, 90, 400, 500,
        NULL, NULL, hInstance, NULL
    );

    if (!hWnd) {
        MessageBoxW(NULL, L"CreateWindow failed!", L"Error", MB_ICONERROR);
        return 1;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}
