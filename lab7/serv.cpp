#include <windows.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include <cstdint>
#include "common.h"


HINSTANCE hInst;
HWND hWndEdit;
HANDLE workerHandle;
bool workerShouldStop = false;
HANDLE hPipe;
LPCWSTR name = L"\\\\.\\pipe\\lab7";
wchar_t buf[1024] = {0};

#define WM_READPIPE (WM_USER + 1)


void EditAppendText(HWND hEdit, const wchar_t *text){
    int len = GetWindowTextLengthW(hEdit);
    SendMessageW(hEdit, EM_SETSEL, (WPARAM)len, (LPARAM)len);

    SendMessageW(hEdit, EM_REPLACESEL, 0, (LPARAM)text);
}

DWORD WINAPI workerThread(LPVOID lpParam){
    HWND hwnd = (HWND)lpParam;

    while(!workerShouldStop){
        bool connected = ConnectNamedPipe(hPipe, NULL);
        if (!connected) {
            DWORD err = GetLastError();

            if(err == ERROR_PIPE_CONNECTED){
                connected = TRUE;
            }
            else if(err == ERROR_NO_DATA){
                continue;
            }
            else{
                if(workerShouldStop) break;
                continue;
            }
        }


        DWORD read = 0;

        if(ReadFile(hPipe, buf, sizeof(buf), &read, NULL)){
            buf[read / sizeof(wchar_t)] = L'\0';
            PostMessage(hwnd, WM_READPIPE, 0, 0);
        }
        else{
            buf[0] = L'\0';
            PostMessage(hwnd, WM_READPIPE, 0, 0);
        }

        DisconnectNamedPipe(hPipe);
    }

    return 0;
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
                case ID_CREATE:
                    EditAppendText(hWndEdit, L"Opening the pipe\r\n");
                    hPipe = CreateNamedPipeW(name, PIPE_ACCESS_INBOUND,
                            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE |
                            PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, 0,
                            sizeof(buf), 5000, NULL);
                    if(hPipe == INVALID_HANDLE_VALUE){
                        EditAppendText(hWndEdit, L"Could open the pipe\r\n");
                        return 0;
                    }
                    EditAppendText(hWndEdit, L"Pipe opened successfully\r\n");

                    workerShouldStop = false;
                    workerHandle = CreateThread(NULL, 0, workerThread, hwnd, 0, NULL);

                    if(!workerHandle){
                        EditAppendText(hWndEdit, L"Could not create worker thread\r\n");
                    }

                    return 0;

                case ID_CLOSE:
                    workerShouldStop = true;
                    CancelSynchronousIo(workerHandle);

                    CloseHandle(hPipe);
                    EditAppendText(hWndEdit, L"Pipe closed!\r\n");

                    CloseHandle(workerHandle);
                    return 0;
            }
            return 0;

        case WM_DESTROY:
            workerShouldStop = true;
            CancelSynchronousIo(workerHandle);

            CloseHandle(hPipe);

            CloseHandle(workerHandle);
            PostQuitMessage(0);
            return 0;

        case WM_READPIPE:
            if(!buf[0]){
                EditAppendText(hWndEdit, L"Could not connect on the pipe, or read the contents\r\n");
                return 0;
            }

            EditAppendText(hWndEdit, L"Received a message, contents:\r\n");
            EditAppendText(hWndEdit, buf);
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
    wc.lpszClassName = L"lab7 server";

    if (!RegisterClassExW(&wc)){
        MessageBoxW(NULL, L"RegisterClassEx failed!", L"Error", MB_ICONERROR);
        return 1;
    }

    HWND hWnd = CreateWindowW(
        L"lab7 server", L"lab7 server",
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


