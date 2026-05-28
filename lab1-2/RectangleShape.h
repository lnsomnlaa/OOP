#pragma once
#include "Shape.h"

class RectangleShape : public Shape {
public:
    RectangleShape(int left, int top, int right, int bottom);
    int left() const { return L; }
    int top() const { return T; }
    int right() const { return R; }
    int bottom() const { return B; }

private:
    int L, T, R, B;
};
