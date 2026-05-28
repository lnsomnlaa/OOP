#pragma once
#include "Shape.h"
#include "ShapeTags.h"

/**
 * "Tetra3d": 6 params = flat triangle (like 2D triangle); 4 params = 3D tetrahedron (cx cy cz edge)
 * projected as wireframe.
 */
class Tetra3DShape : public Shape {
public:
    Tetra3DShape(bool projected3d, int a, int b, int c, int d, int e, int f);
    uint32_t binaryTypeTag() const override { return ShapeTags::Tetra3D; }
    void setFlat(int x1, int y1, int x2, int y2, int x3, int y3);
    void setProjected(int cx, int cy, int cz, int edge);

    bool isProjected3D() const { return projected3d_; }
    int x1() const { return ax1; }
    int y1() const { return ay1; }
    int x2() const { return ax2; }
    int y2() const { return ay2; }
    int x3() const { return ax3; }
    int y3() const { return ay3; }
    double centerX() const { return CX; }
    double centerY() const { return CY; }
    double centerZ() const { return CZ; }
    double edge() const { return edgeLen; }

private:
    bool projected3d_;
    int ax1, ay1, ax2, ay2, ax3, ay3;
    double CX, CY, CZ, edgeLen;
};
