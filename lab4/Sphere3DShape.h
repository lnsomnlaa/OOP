#pragma once
#include "Shape.h"
#include "ShapeTags.h"

/**
 * "Sphere3D": 3 params = flat circle (screen cx, cy, r); 4+ params = cx cy cz r wireframe projection
 */
class Sphere3DShape : public Shape {
public:
    Sphere3DShape(bool projected3d, int a, int b, int c, int d = 0);
    uint32_t binaryTypeTag() const override { return ShapeTags::Sphere3D; }
    void setFlat(int cx, int cy, int r);
    void setProjected(int cx, int cy, int cz, int r);

    bool isProjected3D() const { return projected3d_; }
    int scrCx() const { return scx; }
    int scrCy() const { return scy; }
    int scrR() const { return sr; }
    double centerX() const { return CX; }
    double centerY() const { return CY; }
    double centerZ() const { return CZ; }
    double radius() const { return rad; }

private:
    bool projected3d_;
    int scx, scy, sr;
    double CX, CY, CZ, rad;
};
