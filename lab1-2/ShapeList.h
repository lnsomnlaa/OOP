#pragma once
#include "Shape.h"
#include <windows.h>
#include <vector>

/**
 * Owns shapes and triggers drawing through ShapeRenderer
 */
class ShapeList {
public:
    ShapeList() = default;
    ~ShapeList();
    ShapeList(const ShapeList&) = delete;
    ShapeList& operator=(const ShapeList&) = delete;

    void add(Shape* shape);
    void remove(int index);
    void clear();
    void drawAll(HDC hdc) const;

    const std::vector<Shape*>& items() const { return shapes; }

private:
    std::vector<Shape*> shapes;
};
