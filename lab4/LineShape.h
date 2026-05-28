#pragma once
#include "Shape.h"
#include "ShapeTags.h"

class LineShape : public Shape {
public:
    LineShape(int x1, int y1, int x2, int y2);
    uint32_t binaryTypeTag() const override { return ShapeTags::Line; }
    void setEndpoints(int nx1, int ny1, int nx2, int ny2);
    int x1() const { return ax1; }
    int y1() const { return ay1; }
    int x2() const { return ax2; }
    int y2() const { return ay2; }

private:
    int ax1, ay1, ax2, ay2;
};
