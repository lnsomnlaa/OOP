#pragma once
#include "EllipseShape.h"
#include "ShapeTags.h"

class CircleShape : public EllipseShape {
public:
    CircleShape(int cx, int cy, int radius);
    uint32_t binaryTypeTag() const override { return ShapeTags::Circle; }
};
