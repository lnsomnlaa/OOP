#include "ShapeRenderer.h"
#include "Shape.h"

/**
 * Global table: binary type tag -> draw functor.
 */
std::unordered_map<uint32_t, ShapeRenderer::DrawFn>& ShapeRenderer::drawersByTag() {
    static std::unordered_map<uint32_t, DrawFn> table;
    return table;
}

void ShapeRenderer::registerDrawerForTag(uint32_t binaryTag, DrawFn fn) {
    if (binaryTag == 0 || !fn) {
        return;
    }
    drawersByTag()[binaryTag] = std::move(fn);
}

void ShapeRenderer::draw(HDC hdc, const Shape* shape) {
    if (!hdc || !shape) {
        return;
    }
    const uint32_t tag = shape->binaryTypeTag();
    const auto it = drawersByTag().find(tag);
    if (it != drawersByTag().end()) {
        it->second(hdc, shape);
    }
}
