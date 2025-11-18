#include <windows.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include <cstdint>
#include "common.h"


#define ID_TIMER 3001

HINSTANCE hInst;
HWND hWndEdit;
HANDLE hMailslot;
LPCWSTR mailslotPath = L"\\\\.\\mailslot\\lab5";

void EditAppendText(HWND hEdit, const wchar_t *text){
    int len = GetWindowTextLengthW(hEdit);
    SendMessageW(hEdit, EM_SETSEL, (WPARAM)len, (LPARAM)len);

    SendMessageW(hEdit, EM_REPLACESEL, 0, (LPARAM)text);
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
                case ID_MAILSLOT_CREATE:
                    EditAppendText(hWndEdit, L"Creating mailslot\r\n");
                    hMailslot = CreateMailslotW(mailslotPath, 0, MAILSLOT_WAIT_FOREVER, NULL);
                    if(hMailslot == INVALID_HANDLE_VALUE){
                        EditAppendText(hWndEdit, L"Could not create a mailslot\r\n");
                        return 0;
                    }
                    EditAppendText(hWndEdit, L"Mailslot created successfully!\r\n");
                    SetTimer(hwnd, ID_TIMER, 1000, (TIMERPROC)NULL);
                    return 0;
                case ID_MAILSLOT_CLOSE:
                    CloseHandle(hMailslot);
                    EditAppendText(hWndEdit, L"Mailslot closed!\r\n");
                    return 0;
            }
            return 0;

        case WM_DESTROY:
            CloseHandle(hMailslot);
            PostQuitMessage(0);
            return 0;

        case WM_TIMER:
            switch(wParam){
                case ID_TIMER:
                    DWORD mSize, mCount, bytesRead;
                    bool res = GetMailslotInfo(hMailslot, NULL, &mSize, &mCount, NULL);
                    if(res && mCount){
                        std::wstring buf;
                        buf.resize(mSize);
                        ReadFile(hMailslot, buf.data(), mSize, &bytesRead, NULL);
                        EditAppendText(hWndEdit, L"Read data:\r\n");
                        EditAppendText(hWndEdit, buf.c_str());
                        EditAppendText(hWndEdit, L"\r\n");
                    }
                    return 0;
            }
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
    wc.lpszClassName = L"lab5 server";

    if (!RegisterClassExW(&wc)){
        MessageBoxW(NULL, L"RegisterClassEx failed!", L"Error", MB_ICONERROR);
        return 1;
    }

    HWND hWnd = CreateWindowW(
        L"lab5 server", L"lab5 server",
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


