#include "ShapeList.h"
#include "ShapeRenderer.h"
#include <windows.h>

ShapeList::~ShapeList() {
    clear();
}

void ShapeList::add(Shape* shape) {
    if (shape) {
        shapes.push_back(shape);
    }
}

void ShapeList::clear() {
    for (Shape* s : shapes) {
        delete s;
    }
    shapes.clear();
}

void ShapeList::removeAt(size_t index) {
    if (index >= shapes.size()) {
        return;
    }
    delete shapes[index];
    shapes.erase(shapes.begin() + static_cast<std::ptrdiff_t>(index));
}

void ShapeList::replaceAt(size_t index, Shape* shape) {
    if (index >= shapes.size() || !shape) {
        delete shape;
        return;
    }
    delete shapes[index];
    shapes[index] = shape;
}

void ShapeList::drawAll(HDC hdc) const {
    if (!hdc) {
        return;
    }
    for (const Shape* s : shapes) {
        ShapeRenderer::draw(hdc, s);
    }
}
