#pragma once
#include <windows.h>

class graph{
    private:
        double (*func)(double);
        double min_x;
        double max_x;

    public:
        bool init(double (*func)(double), double min_x, double max_x);
        void plot(HDC hdc, RECT clientRect);
        void plotDB(HDC hdc, RECT clientRect);
};
