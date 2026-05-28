#include "RectangleShape.h"
#include "ShapeFactory.h"
#include "ShapeRenderer.h"
#include <algorithm>
#include <memory>
#include <vector>
#include <windows.h>

RectangleShape::RectangleShape(int l, int t, int r, int b) : L(l), T(t), R(r), B(b) {}

namespace {

class RectCreator : public IShapeCreator {
public:
    Shape* create(const std::vector<int>& p) const override {
        return new RectangleShape(p[0], p[1], p[2], p[3]);
    }
    size_t minParamCount() const override { return 4; }
    const char* commandId() const override { return "rect"; }
};

class RectMouse : public IMouseTool {
public:
    size_t pointCount() const override { return 2; }
    const char* commandId() const override { return "rect"; }
    std::vector<int> toParams(const std::vector<POINT>& pts) const override {
        const int x0 = static_cast<int>(pts[0].x), y0 = static_cast<int>(pts[0].y);
        const int x1 = static_cast<int>(pts[1].x), y1 = static_cast<int>(pts[1].y);
        return {(std::min)(x0, x1), (std::min)(y0, y1), (std::max)(x0, x1), (std::max)(y0, y1)};
    }
};

struct Reg {
    Reg() {
        ShapeRenderer::registerDrawer<RectangleShape>([](HDC hdc, const Shape* s) {
            const RectangleShape* r = static_cast<const RectangleShape*>(s);
            Rectangle(hdc, r->left(), r->top(), r->right(), r->bottom());
        });
        ShapeFactory::registerCreator(std::make_unique<RectCreator>());
        MouseToolRegistry::registerTool(std::make_unique<RectMouse>());
    }
};
static Reg reg;

} // namespace
