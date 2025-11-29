#include <windows.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include <cstdint>
#include <wininet.h>
#include "commonClient.h"

HINSTANCE hInst;
HWND hWndEdit;
LPCWSTR name = L"\\\\.\\pipe\\lab7";

void EditAppendText(HWND hEdit, const wchar_t *text){
    int len = GetWindowTextLengthW(hEdit);
    SendMessageW(hEdit, EM_SETSEL, (WPARAM)len, (LPARAM)len);

    SendMessageW(hEdit, EM_REPLACESEL, 0, (LPARAM)text);
}

void sendToServer(HWND hWnd){
    HANDLE hPipe = CreateFileW(name, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hPipe == INVALID_HANDLE_VALUE){
        EditAppendText(hWndEdit, L"Failed to open pipe\r\n");
        return;
    }

    // SM_SERVERR2 -> The build number if the system is Windows Server 2003 R2; otherwise, 0. 
    // XD
    int osVersion = GetSystemMetrics(SM_SERVERR2);

    int monitorCount = GetSystemMetrics(SM_CMONITORS);

    HDC hdc = GetDC(NULL);
    int colorDepth = GetDeviceCaps(hdc, BITSPIXEL);
    ReleaseDC(NULL, hdc);

    std::wstring data = L"From client 1\r\nSM_SERVERR2: " +
      std::to_wstring(osVersion) + L"\r\nКількість моніторів: " +
      std::to_wstring(monitorCount) + L"\r\nГлибина кольору: " + std::to_wstring(colorDepth);

    EditAppendText(hWndEdit, L"Sending message:\r\n");
    EditAppendText(hWndEdit, data.c_str());
    EditAppendText(hWndEdit, L"\r\n");


    DWORD written = 0;

    if(!WriteFile(hPipe, data.c_str(), data.size() * sizeof(wchar_t), &written, NULL)){
        EditAppendText(hWndEdit, L"WriteFile failed!\r\n");
        CloseHandle(hPipe);
        return;
    }

    EditAppendText(hWndEdit, L"Sent successfully!\r\n");

    CloseHandle(hPipe);
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
    wc.lpszClassName = L"lab7 client1";

    if (!RegisterClassExW(&wc)){
        MessageBoxW(NULL, L"RegisterClassEx failed!", L"Error", MB_ICONERROR);
        return 1;
    }

    HWND hWnd = CreateWindowW(
        L"lab7 client1", L"lab7 client1",
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


