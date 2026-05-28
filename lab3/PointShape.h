#pragma once
#include "EllipseShape.h"
#include "ShapeTags.h"

class PointShape : public EllipseShape {
public:
    explicit PointShape(int x, int y);
    uint32_t binaryTypeTag() const override { return ShapeTags::Point; }
    void setCenter(int x, int y);
};
