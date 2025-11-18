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
unsigned int a;
unsigned int b;
bool inputSuccessA = false;
bool inputSuccessB = false;

void EditAppendText(HWND hEdit, const wchar_t *text){
    int len = GetWindowTextLengthW(hEdit);
    SendMessageW(hEdit, EM_SETSEL, (WPARAM)len, (LPARAM)len);

    SendMessageW(hEdit, EM_REPLACESEL, 0, (LPARAM)text);
}

INT_PTR CALLBACK InputDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam){
    switch(msg){
        case WM_COMMAND:
        switch (LOWORD(wParam)){
            case IDC_OK:
            {
                wchar_t buffer[64];
                if(!GetDlgItemText(hDlg, IDC_TEXTBOX, buffer, 64)){
                    EditAppendText(hWndEdit, L"Invalid input!\r\n");
                    if(inputSuccessA) inputSuccessB = false;
                    else inputSuccessA = false;
                    return 0;
                }
                for(int i = 0; i<64 && buffer[i]; i++){
                    if(!(buffer[i] <= '9' && buffer[i] >= '0')){
                        EditAppendText(hWndEdit, L"Invalid input!\r\n");
                        if(inputSuccessA) inputSuccessB = false;
                        else inputSuccessA = false;
                        return 0;
                    }
                }

                unsigned int num = std::stoi(buffer);

                if(inputSuccessA){
                    b = num;
                    inputSuccessB = true;
                }
                else{
                    a = num;
                    inputSuccessA = true;
                }
                
                EndDialog(hDlg, 1);
                return 0;
            }
            case IDC_CANCEL:
                EndDialog(hDlg, 0);
                return 0;
            }
        break;
    }
    return 0;
}

void sendToServer(HWND hWnd){
    DWORD bytesWritten;

 
    DialogBox((HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), MAKEINTRESOURCE(IDD_INPUT_DIALOGA), hWnd, InputDlgProc);
    if(inputSuccessA){
        DialogBox((HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), MAKEINTRESOURCE(IDD_INPUT_DIALOGB), hWnd, InputDlgProc);
    }

    if(!(inputSuccessA && inputSuccessB)) return;

    std::wstring data = L"From client 3\r\nA = " + std::to_wstring(a) + L"\r\nB = "
      + std::to_wstring(b) + L"\r\nA * B = " + std::to_wstring(a * b) +
      L"\r\n";

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
    wc.lpszClassName = L"lab5 client3";

    if (!RegisterClassExW(&wc)){
        MessageBoxW(NULL, L"RegisterClassEx failed!", L"Error", MB_ICONERROR);
        return 1;
    }

    HWND hWnd = CreateWindowW(
        L"lab5 client3", L"lab5 client3",
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


