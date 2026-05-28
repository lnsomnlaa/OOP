#pragma once
#include <windows.h>
#include <utility>
#include <vector>

struct Projection3D {
    static POINT map(double x, double y, double z);
};

/**
 * Abstract 3D solid: provides projected wireframe segments for display 
 */
class Solid3D {
public:
    virtual ~Solid3D() = default;
    virtual std::vector<std::pair<POINT, POINT>> projectedWireframe() const = 0;

protected:
    double cx = 0;
    double cy = 0;
    double cz = 0;
};

/** Axis-aligned cube: center (cx,cy,cz), edge length. */
class CubeSolid3D : public Solid3D {
public:
    CubeSolid3D(double centerX, double centerY, double centerZ, double edgeLength);

    std::vector<std::pair<POINT, POINT>> projectedWireframe() const override;

private:
    double half = 0;
};

/** Sphere: center and radius; wireframe = three projected great circles. */
class SphereSolid3D : public Solid3D {
public:
    SphereSolid3D(double centerX, double centerY, double centerZ, double radius);

    std::vector<std::pair<POINT, POINT>> projectedWireframe() const override;

private:
    double radius = 0;
};

/** Regular tetrahedron: center and edge length. */
class TetraSolid3D : public Solid3D {
public:
    TetraSolid3D(double centerX, double centerY, double centerZ, double edgeLength);

    std::vector<std::pair<POINT, POINT>> projectedWireframe() const override;

private:
    double edge = 0;
};
