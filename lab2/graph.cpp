#include "graph.hpp"
#include <iostream>
#include <vector>
#include <cfloat>

bool graph::init(double (*func)(double), double min_x, double max_x){
    if(max_x <= min_x){
        std::cout << "invalid bounds in graph init\n";
        return false;
    }
    
    this->func = func;
    this->min_x = min_x;
    this->max_x = max_x;

    return true;
}

void graph::plot(HDC hdc, RECT clientRect){
    int width = clientRect.right - clientRect.left;
    int height = clientRect.bottom - clientRect.top;

    int n_points = width;
    std::vector<POINT> pts(n_points);

    double dx = (max_x - min_x) / (n_points - 1);

    double min_y = DBL_MAX, max_y = -DBL_MAX;
    std::vector<double> values(n_points);

    for (int i = 0; i < n_points; i++) {
        double x = min_x + i * dx;
        double y = func(x);

        values[i] = y;

        if (y < min_y) min_y = y;
        if (y > max_y) max_y = y;
    }

    double scale_y = (double)height / (max_y - min_y);

    // drawing axis if they're visible
    HPEN pen = CreatePen(PS_DASH, 1, RGB(50, 50, 50));
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);

    // X-axis
    if (min_y <= 0 && max_y >= 0) {
        int y0 = clientRect.bottom - (int)((0 - min_y) * scale_y);
        MoveToEx(hdc, clientRect.left, y0, nullptr);
        LineTo(hdc, clientRect.right, y0);
    }

    // Y-axis
    if (min_x <= 0 && max_x >= 0) {
        int x0 = clientRect.left + (int)((0 - min_x) * (width / (max_x - min_x)));
        MoveToEx(hdc, x0, clientRect.top, nullptr);
        LineTo(hdc, x0, clientRect.bottom);
    }

    SelectObject(hdc, oldPen);
    DeleteObject(pen);


    for (int i = 0; i < n_points; i++) {
        int screen_x = clientRect.left + i;
        int screen_y = clientRect.bottom - (int)((values[i] - min_y) * scale_y);
        pts[i] = {screen_x, screen_y};
    }

    pen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
    oldPen = (HPEN)SelectObject(hdc, pen);

    Polyline(hdc, pts.data(), n_points);

    SelectObject(hdc, oldPen);
    DeleteObject(pen);
}
