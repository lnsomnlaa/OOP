#pragma once
#include "Shape.h"
#include "ShapeTags.h"

class TriangleShape : public Shape {
public:
    TriangleShape(int x1, int y1, int x2, int y2, int x3, int y3);
    uint32_t binaryTypeTag() const override { return ShapeTags::Triangle; }
    void setVertices(int x1, int y1, int x2, int y2, int x3, int y3);
    int x1() const { return ax1; }
    int y1() const { return ay1; }
    int x2() const { return ax2; }
    int y2() const { return ay2; }
    int x3() const { return ax3; }
    int y3() const { return ay3; }

private:
    int ax1, ay1, ax2, ay2, ax3, ay3;
};
