#pragma once
#include <windows.h>
#include <functional>
#include <typeindex>
#include <unordered_map>

class Shape;

/**
 * Draws shapes via a type-index registry so core code never switches on concrete figure types
 */
class ShapeRenderer {
public:
    using DrawFn = std::function<void(HDC, const Shape*)>;

    template<typename T>
    static void registerDrawer(DrawFn fn);

    static void draw(HDC hdc, const Shape* shape);

private:
    static std::unordered_map<std::type_index, DrawFn>& drawers();
};

template<typename T>
void ShapeRenderer::registerDrawer(DrawFn fn) {
    drawers()[std::type_index(typeid(T))] = std::move(fn);
}
