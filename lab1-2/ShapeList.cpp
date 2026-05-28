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

void ShapeList::remove(int index) {
    if (index < 0 || index >= static_cast<int>(shapes.size())) return;
    delete shapes[index];
    shapes.erase(shapes.begin() + index);
}

void ShapeList::clear() {
    for (Shape* s : shapes) {
        delete s;
    }
    shapes.clear();
}

void ShapeList::drawAll(HDC hdc) const {
    if (!hdc) {
        return;
    }
    for (const Shape* s : shapes) {
        ShapeRenderer::draw(hdc, s);
    }
}
