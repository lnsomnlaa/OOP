#include "ShapeRenderer.h"
#include "Shape.h"
#include <typeinfo>

std::unordered_map<std::type_index, ShapeRenderer::DrawFn>& ShapeRenderer::drawers() {
    static std::unordered_map<std::type_index, DrawFn> table;
    return table;
}

void ShapeRenderer::draw(HDC hdc, const Shape* shape) {
    if (!hdc || !shape) {
        return;
    }
    const std::type_index key(typeid(*shape));
    const auto it = drawers().find(key);
    if (it != drawers().end()) {
        it->second(hdc, shape);
    }
}