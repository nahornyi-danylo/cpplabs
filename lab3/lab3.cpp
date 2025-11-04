#include <windows.h>
#include <iostream>
#include <stdio.h>
#include <cmath>
#include <cfloat>
#include <list>
#include <cstdlib>
#include <ctime>
#include <vector>
#include "common.h"

// ======================================
// Extra task globals 
#define ARR_LEN 10000
unsigned int nThreads = 1;
int array[ARR_LEN];
std::vector<HANDLE> workerThreadsVector;
unsigned int sum;
HANDLE sumMutex;
HANDLE masterThreadHandle;
unsigned int done;
HANDLE halfdoneEvent;
HANDLE masterThreadEvent;

enum {
  START,
  EXIT,
} masterThreadEventType;

struct workerInfo {
    int id;
    int threadCount;
};

// Main task globals

struct drawInfo {
    unsigned int num;
    POINT pos;
    HANDLE threadHandle;
    bool shouldStop;

    std::list<drawInfo>::iterator selfIterator;
};

std::list<drawInfo> drawList;
HANDLE drawListMutex;
HWND hWnd;

// ======================================

INT_PTR CALLBACK InputDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam){
    switch(msg){
        case WM_COMMAND:
        switch (LOWORD(wParam)){
            case IDC_OK:
            {
                wchar_t buffer[64];
                if(!GetDlgItemText(hDlg, IDC_TEXTBOX, buffer, 64)) return 0;

                for(int i = 0; i<64 && buffer[i]; i++){
                    if(!(buffer[i] <= '9' && buffer[i] >= '0')) return 0;
                }

                unsigned int num = std::stoi(buffer);
                printf("entered %d\n", num);
                if(num > 64 || num < 1){
                    return 0;
                }

                nThreads = num;
                
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

DWORD WINAPI workerThread(LPVOID lpParam){
    unsigned systemId = GetCurrentThreadId();
    workerInfo *info = (workerInfo *)lpParam;
    unsigned localSum = 0;
    unsigned start = (ARR_LEN * info->id) / info->threadCount ;
    unsigned end = (ARR_LEN * (info->id + 1)) / info->threadCount;

    printf("%d started\n", systemId);
    
    if(info->id > info->threadCount / 2){
        WaitForSingleObject(halfdoneEvent, INFINITE);
    }

    for(int i = start; i<end; i++){
        double v = array[i] * 10.12345;
        for (int j = 0; j < 50000; j++){
            v += sin(v) + sqrt(j+1);
        }
        localSum += (unsigned int)v;
    }

    WaitForSingleObject(sumMutex, INFINITE);
    printf("%d accessing a shared resource\n", systemId);
    sum += localSum;
    done++;
    if(done > info->threadCount / 2) SetEvent(halfdoneEvent);
    ReleaseMutex(sumMutex);

    printf("%d finished\n", systemId);
    ExitThread(0);
}

DWORD WINAPI masterThread(LPVOID lpParam){
    bool shouldExit = false;

    while(!shouldExit){
        WaitForSingleObject(masterThreadEvent, INFINITE);
        switch(masterThreadEventType){
            case START:
            {
                int localThreadCount = nThreads;

                for(int i = 0; i<ARR_LEN; i++){
                    array[i] = rand();
                }

                sum = 0;
                done = 0;

                std::vector<workerInfo> info;
                info.reserve(localThreadCount);
                
                for(int i = 0; i<localThreadCount; i++){
                    info.push_back({.id = i, .threadCount = localThreadCount});
                    workerThreadsVector.push_back(CreateThread(NULL, 0, workerThread, &info[i], 0, NULL));
                }

                WaitForMultipleObjects(localThreadCount, workerThreadsVector.data(), TRUE, INFINITE);

                ResetEvent(halfdoneEvent);
                printf("result = %u\n", sum);

                for(HANDLE h : workerThreadsVector){
                    CloseHandle(h);
                }
                workerThreadsVector.clear();

                break;
            }
            case EXIT:
                shouldExit = true;
                break;
        }
    }

    ExitThread(0);
}




double getDistance(POINT a, POINT b){
    return std::sqrt((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y));
}


DWORD WINAPI threadProc(LPVOID lpParam){
    drawInfo *info = (drawInfo *)lpParam;

    while(!info->shouldStop){
        info->num++;
        InvalidateRect(hWnd, NULL, TRUE);
        Sleep(100);
    }

    WaitForSingleObject(drawListMutex, INFINITE);
    drawList.erase(info->selfIterator);
    ReleaseMutex(drawListMutex);

    CloseHandle(info->threadHandle);
    InvalidateRect(hWnd, NULL, TRUE);

    return 0;
}



LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
    switch(msg){
        case WM_COMMAND:
            switch(LOWORD(wParam)){
                case IDM_SHOW_POPUP:
                    printf("current number of threads is %u\n", nThreads);
                    DialogBox((HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), MAKEINTRESOURCE(IDD_INPUT_DIALOG), hWnd, InputDlgProc);
                    break;
                case IDM_START_BUTTON:
                    printf("start button pressed with %u threads\n", nThreads);
                    masterThreadEventType = START;
                    SetEvent(masterThreadEvent);
                    break;
            }
            return 0;

        case WM_DESTROY:
            WaitForSingleObject(drawListMutex, INFINITE);
            for(drawInfo &item : drawList){
                item.shouldStop = true;
            }
            ReleaseMutex(drawListMutex);

            masterThreadEventType = EXIT;
            SetEvent(masterThreadEvent);
            WaitForSingleObject(masterThreadHandle, INFINITE);
            PostQuitMessage(0);
            return 0;

        case WM_LBUTTONDOWN:
        {
            POINT p;
            if(!GetCursorPos(&p) || !ScreenToClient(hWnd, &p)){
              printf("error while getting the cursor position\n");
              return -1;
            }
            printf("mouse 1 was clicked at %d %d\n", p.x, p.y);

            WaitForSingleObject(drawListMutex, INFINITE);
            auto itemIterator = drawList.emplace(drawList.end(), drawInfo{0, p, 0, false});
            ReleaseMutex(drawListMutex);

            drawInfo *item = &(*itemIterator);
            item->selfIterator = itemIterator;
            item->threadHandle = CreateThread(NULL, 0, threadProc, item, CREATE_SUSPENDED, NULL);
            ResumeThread(item->threadHandle);

            return 0;
        }


        case WM_RBUTTONDOWN:
        {
            POINT p;
            if(!GetCursorPos(&p) || !ScreenToClient(hWnd, &p)){
              printf("error while getting the cursor position\n");
              return -1;
            }
            printf("mouse 2 was clicked at %d %d\n", p.x, p.y);

            if(drawList.empty()){
                return 0;
            }

            drawInfo *toDelete = NULL;
            double currentDistance = DBL_MAX;

            WaitForSingleObject(drawListMutex, INFINITE);
            for(drawInfo &item : drawList){
                double t = getDistance(p, item.pos);

                if(t < currentDistance && item.shouldStop == false){
                  currentDistance = t;
                  toDelete = &item;
                }
            }
            ReleaseMutex(drawListMutex);

            if(toDelete){
                toDelete->shouldStop = true;
                printf("deleted thread {%d, %d}, %d\n", toDelete->pos.x, toDelete->pos.y, toDelete->num);
            }

            return 0;
        }

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            WaitForSingleObject(drawListMutex, INFINITE);
            for(drawInfo &item : drawList){
                std::wstring text = L"{" + std::to_wstring(item.pos.x) + L", "
                    + std::to_wstring(item.pos.y) + L"}, " +
                    std::to_wstring(item.num); 

                TextOutW(hdc, item.pos.x, item.pos.y, text.c_str(), text.length());
            }
            ReleaseMutex(drawListMutex);

            EndPaint(hWnd, &ps);
            return 0;
        }

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
    wc.lpszClassName = L"lab3";

    if (!RegisterClassExW(&wc)){
        MessageBoxW(NULL, L"RegisterClassEx failed!", L"Error", MB_ICONERROR);
        return 1;
    }

    hWnd = CreateWindowW(
        L"lab3", L"lab3",
        WS_OVERLAPPEDWINDOW,   
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL, LoadMenu(hInstance, MAKEINTRESOURCE(APP_MENU)), hInstance, NULL     
    );

    if (!hWnd){
        MessageBoxW(NULL, L"CreateWindow failed!", L"Error", MB_ICONERROR);
        return 1;
    }

    drawListMutex = CreateMutex(NULL, FALSE, NULL);
    sumMutex = CreateMutex(NULL, FALSE, NULL);
    halfdoneEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    masterThreadEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    masterThreadHandle = CreateThread(NULL, 0, masterThread, NULL, 0, NULL);
    srand(time(NULL));

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0)){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    CloseHandle(drawListMutex);
    CloseHandle(sumMutex);
    CloseHandle(halfdoneEvent);
    CloseHandle(masterThreadEvent);
    CloseHandle(masterThreadHandle);

    return (int)msg.wParam;
}


