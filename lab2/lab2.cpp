#include <windows.h>
#include <cmath> // Для функції sin()

// Процедура вікна, яка обробляє всі події
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Головна функція, точка входу в програму.
// Змінено WinMain на wWinMain для сумісності з Unicode-компіляцією.
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"SineWaveWindowClass";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;       // Функція обробки подій
    wc.hInstance = hInstance;       // Дескриптор екземпляра
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); // Білий фон
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);      // Стандартний курсор

    if (!RegisterClass(&wc)) {
        MessageBox(NULL, L"Window Registration Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    HWND hwnd = CreateWindowEx(
        0,                      // Додаткові стилі
        CLASS_NAME,             // Ім'я класу вікна
        L"Графік функції y = sin(x)", // Заголовок вікна
        WS_OVERLAPPEDWINDOW,    // Стиль вікна

        // Розмір та позиція
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,

        NULL,       // Батьківське вікно
        NULL,       // Меню
        hInstance,  // Дескриптор екземпляра
        NULL        // Додаткові дані
    );

    if (hwnd == NULL) {
        MessageBox(NULL, L"Window Creation Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Головний цикл повідомлень
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

// Функція для відмальовування графіка
void PaintGraph(HDC hdc) {
    // --- Подвійна буферизація для уникнення мерехтіння ---
    RECT clientRect;
    GetClientRect(WindowFromDC(hdc), &clientRect);
    int width = clientRect.right - clientRect.left;
    int height = clientRect.bottom - clientRect.top;

    // Якщо вікно згорнуте або занадто мале, нічого не малюємо
    if (width == 0 || height == 0) return;

    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBitmap = CreateCompatibleBitmap(hdc, width, height);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

    // Заливаємо фон білим кольором
    FillRect(memDC, &clientRect, (HBRUSH)(COLOR_WINDOW + 1));

    // --- Визначення діапазонів та масштабу ---
    const double x_min = -10.0;
    const double x_max = 10.0;
    const double y_min = -1.05; // Беремо трохи більше, щоб графік не торкався країв
    const double y_max = 1.05;

    // --- Малювання осей координат ---
    HPEN hAxisPen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200)); // Світло-сірий олівець
    HPEN hOldPen = (HPEN)SelectObject(memDC, hAxisPen);

    // Вісь Y (лінія x=0)
    int x0_screen = (int)((0.0 - x_min) / (x_max - x_min) * width);
    if (x0_screen >= 0 && x0_screen < width) {
        MoveToEx(memDC, x0_screen, 0, NULL);
        LineTo(memDC, x0_screen, height);
    }

    // Вісь X (лінія y=0)
    int y0_screen = (int)((y_max - 0.0) / (y_max - y_min) * height);
    if (y0_screen >= 0 && y0_screen < height) {
        MoveToEx(memDC, 0, y0_screen, NULL);
        LineTo(memDC, width, y0_screen);
    }

    // --- Малювання графіка функції sin(x) ---
    HPEN hGraphPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 255)); // Синій олівець товщиною 2 пікселі
    SelectObject(memDC, hGraphPen);

    // Встановлюємо початкову точку
    double first_x = x_min;
    double first_y = sin(first_x);
    int first_screen_x = 0;
    int first_screen_y = (int)((y_max - first_y) / (y_max - y_min) * height);
    MoveToEx(memDC, first_screen_x, first_screen_y, NULL);

    // Малюємо лінію, проходячи по кожному пікселю по ширині вікна
    for (int screen_x = 1; screen_x < width; ++screen_x) {
        // Перетворюємо екранну координату X назад у математичну
        double world_x = x_min + (screen_x / (double)(width - 1)) * (x_max - x_min);
        double world_y = sin(world_x);

        // Перетворюємо математичну координату Y в екранну
        int screen_y = (int)((y_max - world_y) / (y_max - y_min) * height);

        LineTo(memDC, screen_x, screen_y);
    }

    // --- Копіювання з буфера на екран ---
    BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

    // Очищення ресурсів
    SelectObject(memDC, hOldPen);
    SelectObject(memDC, hOldBitmap);
    DeleteObject(hAxisPen);
    DeleteObject(hGraphPen);
    DeleteObject(memBitmap);
    DeleteDC(memDC);
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        // Ця подія викликається щоразу, коли вікно потрібно перемалювати
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            PaintGraph(hdc); // Викликаємо нашу функцію малювання
            EndPaint(hwnd, &ps);
        }
        return 0;

        // Повідомлення про зміну розміру вікна
        case WM_SIZE: {
            // Повідомляємо вікну, що його вміст застарів і потребує перемальовування
            InvalidateRect(hwnd, NULL, FALSE);
        }
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}