#pragma once
#include "Shape.h"
#include <windows.h>
#include <vector>

class PolygonShape : public Shape {
public:
    explicit PolygonShape(std::vector<POINT> points);

    const std::vector<POINT>& points() const { return pts; }

private:
    std::vector<POINT> pts;
};

