#include <windows.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include <cstdint>
#include <wininet.h>
#include "commonClient.h"

HINSTANCE hInst;
HWND hWndEdit;

void EditAppendText(HWND hEdit, const wchar_t *text){
    int len = GetWindowTextLengthW(hEdit);
    SendMessageW(hEdit, EM_SETSEL, (WPARAM)len, (LPARAM)len);

    SendMessageW(hEdit, EM_REPLACESEL, 0, (LPARAM)text);
}

void sendToServer(HWND hWnd){
    HANDLE hMapFile = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, L"lab6mmap");
    if (!hMapFile){
        EditAppendText(hWndEdit, L"Could not open file mapping!\r\n");
        return;
    }

    LPVOID buf = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 512);
    if (!buf){
        EditAppendText(hWndEdit, L"MapViewOfFile failed!\r\n");
        CloseHandle(hMapFile);
        return;
    }

    HANDLE readEvent = OpenEventW(EVENT_MODIFY_STATE, FALSE, L"lab6readEvent");
    if (!readEvent){
        EditAppendText(hWndEdit, L"Could not open event!\r\n");
        UnmapViewOfFile(buf);
        CloseHandle(hMapFile);
        return;
    }

    HANDLE mutex = OpenMutexW(SYNCHRONIZE, FALSE, L"lab6sharedMutex");
    if (!mutex){
        EditAppendText(hWndEdit, L"Could not open mutex!\r\n");
        CloseHandle(readEvent);
        UnmapViewOfFile(buf);
        CloseHandle(hMapFile);
        return;
    }

    WaitForSingleObject(mutex, INFINITE);

    int menuWidth = GetSystemMetrics(SM_CYMENU);
    int mousePresent = GetSystemMetrics(SM_MOUSEPRESENT);

    HDC hdc = GetDC(NULL);
    int width = GetDeviceCaps(hdc, HORZRES);
    ReleaseDC(NULL, hdc);

    std::wstring data = L"From client 1\r\nШирина полоси меню: " +
      std::to_wstring(menuWidth) + L"\r\nНаявність миші: " +
      std::to_wstring(mousePresent) + L"\r\nШирина екрану: " + std::to_wstring(width);

    EditAppendText(hWndEdit, L"Sending message:\r\n");
    EditAppendText(hWndEdit, data.c_str());
    EditAppendText(hWndEdit, L"\r\n");

    wcscpy_s((wchar_t*)buf, 512 / sizeof(wchar_t), data.c_str());

    SetEvent(readEvent);
    ReleaseMutex(mutex);

    EditAppendText(hWndEdit, L"Sent successfully!\r\n");

    CloseHandle(mutex);
    CloseHandle(readEvent);
    UnmapViewOfFile(buf);
    CloseHandle(hMapFile);
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
    wc.lpszClassName = L"lab6 client1";

    if (!RegisterClassExW(&wc)){
        MessageBoxW(NULL, L"RegisterClassEx failed!", L"Error", MB_ICONERROR);
        return 1;
    }

    HWND hWnd = CreateWindowW(
        L"lab6 client1", L"lab6 client1",
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


