#pragma once
#include <cstdint>

/**
 * Abstract 2D figure model. Rendering is not part of the shape
 */
class Shape {
public:
    virtual ~Shape() = default;
    /** Stable id for binary serialization (see ShapeTags). */
    virtual uint32_t binaryTypeTag() const = 0;
};