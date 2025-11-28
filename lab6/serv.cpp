#include <windows.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include <cstdint>
#include "common.h"


HINSTANCE hInst;
HWND hWndEdit;
HANDLE hMapFile;
HANDLE workerHandle;
HANDLE readEvent;
HANDLE mutex;
LPVOID buf = NULL;

#define WM_READMMAP (WM_USER + 1)

void EditAppendText(HWND hEdit, const wchar_t *text){
    int len = GetWindowTextLengthW(hEdit);
    SendMessageW(hEdit, EM_SETSEL, (WPARAM)len, (LPARAM)len);

    SendMessageW(hEdit, EM_REPLACESEL, 0, (LPARAM)text);
}

DWORD WINAPI workerThread(LPVOID lpParam){
    HWND hwnd = (HWND)lpParam;

    while(1){
        if(WaitForSingleObject(readEvent, INFINITE) == WAIT_OBJECT_0){
            if(WaitForSingleObject(mutex, INFINITE) == WAIT_OBJECT_0){
                buf = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
                PostMessage(hwnd, WM_READMMAP, 0, 0);
                ReleaseMutex(mutex);
            }
            else break;
        }
        else break;
    }

    return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
    switch(msg){
        case WM_CREATE:
            hWndEdit = CreateWindowW(TEXT("EDIT"), NULL, WS_CHILD | WS_VISIBLE
                | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL |
                ES_READONLY, 0, 0, 580, 420, hwnd, NULL, hInst, NULL);

            readEvent = CreateEventW(NULL, FALSE, FALSE, L"lab6readEvent");
            if(!readEvent){
                MessageBoxW(hwnd, L"Could not create event", L"Error", MB_ICONERROR);
                return -1;
            }

            workerHandle = CreateThread(NULL, 0, workerThread, hwnd, 0, NULL);

            if(!workerHandle){
                MessageBoxW(hwnd, L"Could not create event", L"Error", MB_ICONERROR);
                return -1;
            }

            mutex = CreateMutexW(NULL, FALSE, L"lab6sharedMutex");
            if(!mutex){
                MessageBoxW(hwnd, L"Could not create mutex", L"Error", MB_ICONERROR);
                return -1;
            }

            return 0;

        case WM_COMMAND:
            switch(LOWORD(wParam)){
                case ID_CREATE:
                    EditAppendText(hWndEdit, L"Creating file mapping\r\n");
                    hMapFile = CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 512, L"lab6mmap");
                    if(hMapFile == INVALID_HANDLE_VALUE || hMapFile == NULL){
                        EditAppendText(hWndEdit, L"Could not create a file mapping\r\n");
                        return 0;
                    }
                    EditAppendText(hWndEdit, L"File mapping created successfully!\r\n");

                    return 0;

                case ID_CLOSE:
                    CloseHandle(hMapFile);
                    EditAppendText(hWndEdit, L"File closed!\r\n");
                    return 0;
            }
            return 0;

        case WM_DESTROY:
            CloseHandle(readEvent);
            CloseHandle(mutex);
            CloseHandle(workerHandle);

            if(buf){
                UnmapViewOfFile(buf);
            }

            CloseHandle(hMapFile);
            PostQuitMessage(0);
            return 0;

        case WM_READMMAP:
            EditAppendText(hWndEdit, L"Notified of file change, trying to read!\r\n");

            if(!buf){
                EditAppendText(hWndEdit, L"Could not map view of file!\r\n");
                return 0;
            }

            EditAppendText(hWndEdit, L"Contents:\r\n");
            EditAppendText(hWndEdit, LPCWSTR(buf));
            EditAppendText(hWndEdit, L"\r\n");

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
    wc.lpszClassName = L"lab6 server";

    if (!RegisterClassExW(&wc)){
        MessageBoxW(NULL, L"RegisterClassEx failed!", L"Error", MB_ICONERROR);
        return 1;
    }

    HWND hWnd = CreateWindowW(
        L"lab6 server", L"lab6 server",
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


