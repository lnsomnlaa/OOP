#include "EllipseShape.h"
#include "ShapeFactory.h"
#include "ShapeRenderer.h"
#include "ShapeTags.h"
#include <algorithm>
#include <memory>
#include <vector>
#include <windows.h>

EllipseShape::EllipseShape(int l, int t, int r, int b) : L(l), T(t), R(r), B(b) {}

void EllipseShape::setBounds(int l, int t, int r, int b) {
    L = l;
    T = t;
    R = r;
    B = b;
}

namespace {

class EllipseCreator : public IShapeCreator {
public:
    Shape* create(const std::vector<int>& p) const override {
        return new EllipseShape(p[0], p[1], p[2], p[3]);
    }
    size_t minParamCount() const override { return 4; }
    const char* commandId() const override { return "ellipse"; }
};

class EllipseMouse : public IMouseTool {
public:
    size_t pointCount() const override { return 2; }
    const char* commandId() const override { return "ellipse"; }
    std::vector<int> toParams(const std::vector<POINT>& pts) const override {
        const int x0 = static_cast<int>(pts[0].x), y0 = static_cast<int>(pts[0].y);
        const int x1 = static_cast<int>(pts[1].x), y1 = static_cast<int>(pts[1].y);
        return {(std::min)(x0, x1), (std::min)(y0, y1), (std::max)(x0, x1), (std::max)(y0, y1)};
    }
};

struct Reg {
    Reg() {
        ShapeRenderer::registerDrawerForTag(ShapeTags::Ellipse, [](HDC hdc, const Shape* s) {
            const EllipseShape* e = static_cast<const EllipseShape*>(s);
            Ellipse(hdc, e->left(), e->top(), e->right(), e->bottom());
        });
        ShapeFactory::registerCreator(std::make_unique<EllipseCreator>());
        MouseToolRegistry::registerTool(std::make_unique<EllipseMouse>());
    }
};
static Reg reg;

} // namespace
