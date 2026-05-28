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
    void clear();
    void drawAll(HDC hdc) const;

    size_t count() const { return shapes.size(); }
    Shape* at(size_t index) { return index < shapes.size() ? shapes[index] : nullptr; }
    const Shape* at(size_t index) const { return index < shapes.size() ? shapes[index] : nullptr; }
    void removeAt(size_t index);
    /** Replaces shape at index; deletes previous pointer. */
    void replaceAt(size_t index, Shape* shape);

    const std::vector<Shape*>& items() const { return shapes; }

private:
    std::vector<Shape*> shapes;
};
