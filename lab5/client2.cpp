#include <windows.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include <cstdint>
#include <wininet.h>
#include "commonClient.h"

HINSTANCE hInst;
HWND hWndEdit;
HANDLE hMailslot;
LPCWSTR mailslotPath = L"\\\\.\\mailslot\\lab5";

void EditAppendText(HWND hEdit, const wchar_t *text){
    int len = GetWindowTextLengthW(hEdit);
    SendMessageW(hEdit, EM_SETSEL, (WPARAM)len, (LPARAM)len);

    SendMessageW(hEdit, EM_REPLACESEL, 0, (LPARAM)text);
}

void sendToServer(HWND hWnd){
    DWORD bytesWritten;

    int cyEdge = GetSystemMetrics(SM_CYEDGE);
    int cxEdge = GetSystemMetrics(SM_CXEDGE);

    HDC hdc = GetDC(NULL);
    int dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
    ReleaseDC(NULL, hdc);

    std::wstring data = L"From client 2\r\nВисота рядка стану = " +
      std::to_wstring(cyEdge) + L"\r\nШирина панелі повідомлень = " +
      std::to_wstring(cxEdge) + L"\r\nHorizontal DPI = " +
      std::to_wstring(dpiX);

    EditAppendText(hWndEdit, L"Sending message:\r\n");
    EditAppendText(hWndEdit, data.c_str());
    EditAppendText(hWndEdit, L"\r\n");

    uint32_t size = data.size() * sizeof(wchar_t);
    hMailslot = CreateFile(mailslotPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if(hMailslot == INVALID_HANDLE_VALUE){
        EditAppendText(hWndEdit, L"Could not open the mailslot\r\n");
        return;
    }
    WriteFile(hMailslot, data.data(), size, &bytesWritten, NULL);
    EditAppendText(hWndEdit, L"Sent successfully!\r\n");
    CloseHandle(hMailslot);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
    switch(msg){
        case WM_CREATE:
            hWndEdit = CreateWindowW(TEXT("EDIT"), NULL, WS_CHILD | WS_VISIBLE
                    | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL |
                    ES_READONLY, 0, 0, 580, 420, hwnd, NULL, hInst, NULL);

            return 0;
        case WM_COMMAND:
            switch(LOWORD(wParam)){
                case ID_SEND:
                    sendToServer(hwnd);
                    return 0;
            }
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow){
    WNDCLASSEXW wc = {0};
    wc.cbSize        = sizeof(WNDCLASSEXW);
    wc.style         = CS_VREDRAW | CS_HREDRAW;
    wc.lpfnWndProc   = WndProc;       
    wc.hInstance     = hInstance;      
    wc.hCursor       = LoadCursorW(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); 
    wc.lpszClassName = L"lab5 client2";

    if (!RegisterClassExW(&wc)){
        MessageBoxW(NULL, L"RegisterClassEx failed!", L"Error", MB_ICONERROR);
        return 1;
    }

    HWND hWnd = CreateWindowW(
        L"lab5 client2", L"lab5 client2",
        WS_OVERLAPPEDWINDOW,   
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 480,
        NULL, LoadMenu(hInstance, MAKEINTRESOURCE(APP_MENU)), hInstance, NULL     
    );

    if (!hWnd){
        MessageBoxW(NULL, L"CreateWindow failed!", L"Error", MB_ICONERROR);
        return 1;
    }

    hInst = hInstance;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0)){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}


