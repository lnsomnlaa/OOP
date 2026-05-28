#pragma once
#include "Shape.h"
#include "ShapeTags.h"

/**
 * "Cube3D" tool: mouse / 4 params = flat rectangle (2D stand-in); 5+ params = projected 3D wireframe
 * (cx cy cz edge [ignored...]).
 */
class Cube3DShape : public Shape {
public:
    Cube3DShape(bool projected3d, int a, int b, int c, int d);
    uint32_t binaryTypeTag() const override { return ShapeTags::Cube3D; }
    void setFlat(int left, int top, int right, int bottom);
    void setProjected(int cx, int cy, int cz, int edge);

    bool isProjected3D() const { return projected3d_; }
    int left() const { return L; }
    int top() const { return T; }
    int right() const { return R; }
    int bottom() const { return B; }
    double centerX() const { return CX; }
    double centerY() const { return CY; }
    double centerZ() const { return CZ; }
    double edge() const { return edgeLen; }

private:
    bool projected3d_;
    int L, T, R, B;
    double CX, CY, CZ, edgeLen;
};
