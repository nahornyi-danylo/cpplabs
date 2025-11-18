#include <windows.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include <cstdint>
#include "common.h"


#define NCLIENTS 3
#define PORT 12345
#define WSA_ACCEPT (WM_USER + 0)
#define WSA_NETEVENT (WM_USER + 1)

SOCKET serverSocket = INVALID_SOCKET;
SOCKET clients[NCLIENTS];
SOCKADDR_IN clientInfo[NCLIENTS];
HWND hWndEdit;
HINSTANCE hInst;
WSADATA wsaData;
WORD wVersionRequested = MAKEWORD(2, 2);
int currClientId = 0;

void EditAppendText(HWND hEdit, const wchar_t *text) {
    int len = GetWindowTextLengthW(hEdit);
    SendMessageW(hEdit, EM_SETSEL, (WPARAM)len, (LPARAM)len);

    SendMessageW(hEdit, EM_REPLACESEL, 0, (LPARAM)text);
}

void serverStart(HWND hWnd){
    if(WSAStartup(wVersionRequested, &wsaData)){
        MessageBoxW(hWnd, L"WSAStartup failed", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    if(serverSocket != INVALID_SOCKET){
        MessageBoxW(hWnd, L"Server already started", L"Info", MB_OK | MB_ICONINFORMATION);
        return;
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(serverSocket == INVALID_SOCKET){
        MessageBoxW(hWnd, L"Could not create server socket", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    SOCKADDR_IN serverInfo;
    serverInfo.sin_family = AF_INET;
    serverInfo.sin_port = htons(PORT);
    serverInfo.sin_addr.s_addr = INADDR_ANY;

    if(bind(serverSocket, (LPSOCKADDR)&serverInfo, sizeof(serverInfo)) == SOCKET_ERROR){
        closesocket(serverSocket);
        MessageBoxW(hWnd, L"Could not bind to server socket", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    if(listen(serverSocket, 4) == SOCKET_ERROR){
        closesocket(serverSocket);
        MessageBoxW(hWnd, L"Error on listen", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    if(WSAAsyncSelect(serverSocket, hWnd, WSA_ACCEPT, FD_ACCEPT)){
        closesocket(serverSocket);
        MessageBoxW(hWnd, L"WSAAsyncSelect failed", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    EditAppendText(hWndEdit, L"Sever started\r\n");
}

void serverStop(HWND hWnd){
    WSAAsyncSelect(serverSocket, hWnd, 0, 0);
    if(serverSocket != INVALID_SOCKET){
        closesocket(serverSocket);
        serverSocket = INVALID_SOCKET;
        WSACleanup();
        EditAppendText(hWndEdit, L"Server stopped\r\n");
    }
}

bool acceptClient(HWND hWnd, int i){
    int len = sizeof(clientInfo[i]);
    clients[i] = accept(serverSocket, (LPSOCKADDR)&clientInfo[i], &len);
    if(clients[i] != INVALID_SOCKET){
        if(!WSAAsyncSelect(clients[i], hWnd, WSA_NETEVENT, FD_READ | FD_CLOSE)){
            return true;
        }
        else closesocket(clients[i]);
    }

    return false;
}

void onAccept(HWND hWnd, LPARAM lParam){
    if(WSAGETSELECTERROR(lParam)){
        MessageBoxW(hWnd, L"Error on accept", L"Error", MB_OK | MB_ICONERROR);
        return;
    }
    if(currClientId > 2){
        MessageBoxW(hWnd, L"Only 3 clients allowed per program run", L"Error", MB_OK | MB_ICONERROR);
        return;
    }
    if(!acceptClient(hWnd, currClientId)){
        MessageBoxW(hWnd, L"Couldn't connect client", L"Error", MB_OK | MB_ICONERROR);
        return;
    }
    currClientId++;

    const char *ip = inet_ntoa(clientInfo[currClientId-1].sin_addr);
    std::string ipStr(ip);
    std::wstring wipStr(ipStr.begin(), ipStr.end());
    std::wstring portStr = std::to_wstring(ntohs(clientInfo[currClientId-1].sin_port));

    std::wstring s = L"Client " + std::to_wstring(currClientId) +
                 L" connected\r\n IP = " + wipStr +
                 L"\r\n Port = " + portStr + L"\r\n";;

    EditAppendText(hWndEdit, s.c_str());
}

bool recvN(SOCKET s, void *buf, int len){
    char *p = (char *)buf;

    while(len > 0){
        int n = recv(s, p, len, 0);
        if(n <= 0) return false;
        p += n;
        len -= n;
    }

    return true;
}

void onEvent(HWND hWnd, WPARAM wParam, LPARAM lParam){
    int id = -1;
    int n;
    std::wstring s;

    if((SOCKET)wParam == clients[0]) id = 0;
    else if((SOCKET)wParam == clients[1]) id = 1;
    else if((SOCKET)wParam == clients[2]) id = 2;
    if(id == -1) return;

    switch(WSAGETSELECTEVENT(lParam)){
        case FD_READ:
            {
                uint32_t size;
                if(!recvN((SOCKET)wParam, &size, 4)) return;

                s = L"Received message from client " + std::to_wstring(id + 1) + L"\r\n";
                EditAppendText(hWndEdit, s.c_str());

                std::wstring buf;
                buf.resize(size + 1);
                if(!recvN((SOCKET)wParam, (char *)&buf[0], size)) return;
                else{
                    int count = size / sizeof(wchar_t);
                    buf[count] = L'\0';
                    EditAppendText(hWndEdit, buf.c_str());
                    EditAppendText(hWndEdit, L"\r\n");
                }

                break;
            }
        case FD_CLOSE:
            WSAAsyncSelect(clients[id], hWnd, 0, 0);
            closesocket(clients[id]);
            s = L"Client " + std::to_wstring(id + 1) + L" disconnected\r\n" ;
            EditAppendText(hWndEdit, s.c_str());
            break;
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
                case ID_SERVER_START:
                    serverStart(hwnd);
                    return 0;
                case ID_SERVER_STOP:
                    serverStop(hwnd);
                    return 0;
            }
            return 0;

        case WM_DESTROY:
            WSACleanup();
            PostQuitMessage(0);
            return 0;

        case WSA_ACCEPT:
            onAccept(hwnd, lParam);
            return 0;

        case WSA_NETEVENT:
            onEvent(hwnd, wParam, lParam);
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
    wc.lpszClassName = L"lab4 server";

    if (!RegisterClassExW(&wc)){
        MessageBoxW(NULL, L"RegisterClassEx failed!", L"Error", MB_ICONERROR);
        return 1;
    }

    HWND hWnd = CreateWindowW(
        L"lab4 server", L"lab4 server",
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


