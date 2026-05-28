#pragma once
#include "Shape.h"

class EllipseShape : public Shape {
public:
    EllipseShape(int left, int top, int right, int bottom);
    int left() const { return L; }
    int top() const { return T; }
    int right() const { return R; }
    int bottom() const { return B; }

protected:
    int L, T, R, B;
};
