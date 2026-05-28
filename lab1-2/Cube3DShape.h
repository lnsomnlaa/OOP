#pragma once
#include "Shape.h"

/**
 * "Cube3D" tool: mouse / 4 params = flat rectangle (2D stand-in); 5+ params = projected 3D wireframe
 * (cx cy cz edge [ignored...]).
 */
class Cube3DShape : public Shape {
public:
    Cube3DShape(bool projected3d, int a, int b, int c, int d);

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
