#include <windows.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include <cstdint>
#include <wininet.h>
#include "commonClient.h"


#define PORT 12345
#define WSA_NETEVENT (WM_USER + 1)

SOCKET clientSocket = INVALID_SOCKET;
SOCKADDR_IN serverInfo;

HWND hWndEdit;
HINSTANCE hInst;
WSADATA wsaData;
WORD wVersionRequested = MAKEWORD(2, 2);

void EditAppendText(HWND hEdit, const wchar_t *text) {
    int len = GetWindowTextLengthW(hEdit);
    SendMessageW(hEdit, EM_SETSEL, (WPARAM)len, (LPARAM)len);

    SendMessageW(hEdit, EM_REPLACESEL, 0, (LPARAM)text);
}

void connectToServer(HWND hWnd){
    if(WSAStartup(wVersionRequested, &wsaData)){
        MessageBoxW(hWnd, L"WSAStartup failed", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(clientSocket == INVALID_SOCKET){
        MessageBoxW(hWnd, L"Could not create server socket", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    PHOSTENT p = gethostbyname("localhost");
    if(!p){
        closesocket(clientSocket);
        return;
    }

    serverInfo.sin_family = AF_INET;
    serverInfo.sin_port = htons(PORT);
    serverInfo.sin_addr = *(struct in_addr *)p->h_addr;

    if(connect(clientSocket, (PSOCKADDR)&serverInfo, sizeof(serverInfo)) == SOCKET_ERROR){
        closesocket(clientSocket);
        MessageBoxW(hWnd, L"Could not connect", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    if(WSAAsyncSelect(clientSocket, hWnd, WSA_NETEVENT, FD_CLOSE)){
        MessageBoxW(hWnd, L"WSAAsyncSelect failed", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    EditAppendText(hWndEdit, L"Connected\r\n");
}

void disconnectFromServer(HWND hWnd){
    WSAAsyncSelect(clientSocket, hWnd, 0, 0);
    if(clientSocket != INVALID_SOCKET){
        closesocket(clientSocket);
        clientSocket = INVALID_SOCKET;
        WSACleanup();
        EditAppendText(hWndEdit, L"Disconnected\r\n");
    }
}

void sendToServer(HWND hWnd){
    int coreCount;
    bool networkState;

    // SM_DIGITIZER is both deprecated, and doesn't yield the number of cores
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    coreCount = info.dwNumberOfProcessors;
    // SM_NETCAPTURE doesn't exist anymore
    networkState = InternetGetConnectedState(NULL, 0);
    HDC hdc = GetDC(NULL);
    int width = GetDeviceCaps(hdc, HORZRES);
    ReleaseDC(NULL, hdc);

    std::wstring data = L"Core count = " + std::to_wstring(coreCount) + L"\r\nNetwork state = " + std::to_wstring(networkState) + L"\r\nHORZRES = " + std::to_wstring(width);

    EditAppendText(hWndEdit, L"Sending message:\r\n");
    EditAppendText(hWndEdit, data.c_str());
    EditAppendText(hWndEdit, L"\r\n");

    uint32_t size = data.size() * sizeof(wchar_t);
    if(send(clientSocket, (const char *)&size, 4, 0) != SOCKET_ERROR &&
       send(clientSocket, (const char *)data.data(), size, 0) != SOCKET_ERROR){
        EditAppendText(hWndEdit, L"Successfully sent\r\n");
    }
    else{
        EditAppendText(hWndEdit, L"Could not send\r\n");
    }
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
                case ID_CONNECT:
                    connectToServer(hwnd);
                    return 0;
                case ID_SEND:
                    sendToServer(hwnd);
                    return 0;
                case ID_DISCONNECT:
                    disconnectFromServer(hwnd);
                    return 0;
            }
            return 0;

        case WM_DESTROY:
            WSACleanup();
            PostQuitMessage(0);
            return 0;

        case WSA_NETEVENT:
            if(WSAGETSELECTEVENT(lParam) == FD_CLOSE){
                EditAppendText(hWndEdit, L"Server closed\r\n");
                closesocket(clientSocket);
                WSAAsyncSelect(clientSocket, hwnd, 0, 0);
                WSACleanup();
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
    wc.lpszClassName = L"lab4 client1";

    if (!RegisterClassExW(&wc)){
        MessageBoxW(NULL, L"RegisterClassEx failed!", L"Error", MB_ICONERROR);
        return 1;
    }

    HWND hWnd = CreateWindowW(
        L"lab4 client1", L"lab4 client1",
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


