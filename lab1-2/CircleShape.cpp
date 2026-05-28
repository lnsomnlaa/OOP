#include "CircleShape.h"
#include "ShapeFactory.h"
#include "ShapeRenderer.h"
#include <cmath>
#include <memory>
#include <vector>
#include <windows.h>

CircleShape::CircleShape(int cx, int cy, int r) : EllipseShape(cx - r, cy - r, cx + r, cy + r) {}

namespace {

class CircleCreator : public IShapeCreator {
public:
    Shape* create(const std::vector<int>& p) const override {
        return new CircleShape(p[0], p[1], p[2]);
    }
    size_t minParamCount() const override { return 3; }
    const char* commandId() const override { return "circle"; }
};

class CircleMouse : public IMouseTool {
public:
    size_t pointCount() const override { return 2; }
    const char* commandId() const override { return "circle"; }
    std::vector<int> toParams(const std::vector<POINT>& pts) const override {
        const int cx = static_cast<int>(pts[0].x), cy = static_cast<int>(pts[0].y);
        const long dx = pts[1].x - cx, dy = pts[1].y - cy;
        int r = static_cast<int>(std::sqrt(static_cast<double>(dx * dx + dy * dy)) + 0.5);
        if (r < 1) {
            r = 1;
        }
        return {cx, cy, r};
    }
};

struct Reg {
    Reg() {
        ShapeRenderer::registerDrawer<CircleShape>([](HDC hdc, const Shape* s) {
            const CircleShape* c = static_cast<const CircleShape*>(s);
            Ellipse(hdc, c->left(), c->top(), c->right(), c->bottom());
        });
        ShapeFactory::registerCreator(std::make_unique<CircleCreator>());
        MouseToolRegistry::registerTool(std::make_unique<CircleMouse>());
    }
};
static Reg reg;

} // namespace
