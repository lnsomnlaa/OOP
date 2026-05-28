#pragma once
#include "Shape.h"
#include "ShapeTags.h"

class RectangleShape : public Shape {
public:
    RectangleShape(int left, int top, int right, int bottom);
    uint32_t binaryTypeTag() const override { return ShapeTags::Rect; }
    void setBounds(int left, int top, int right, int bottom);
    int left() const { return L; }
    int top() const { return T; }
    int right() const { return R; }
    int bottom() const { return B; }

private:
    int L, T, R, B;
};
