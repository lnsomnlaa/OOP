#include "PointShape.h"
#include "ShapeFactory.h"
#include "ShapeRenderer.h"
#include <memory>
#include <vector>
#include <windows.h>

PointShape::PointShape(int x, int y) : EllipseShape(x - 2, y - 2, x + 2, y + 2) {}

void PointShape::setCenter(int x, int y) {
    setBounds(x - 2, y - 2, x + 2, y + 2);
}

namespace {

class PointCreator : public IShapeCreator {
public:
    Shape* create(const std::vector<int>& p) const override {
        return new PointShape(p[0], p[1]);
    }
    size_t minParamCount() const override { return 2; }
    const char* commandId() const override { return "point"; }
};

class PointMouse : public IMouseTool {
public:
    size_t pointCount() const override { return 1; }
    const char* commandId() const override { return "point"; }
    std::vector<int> toParams(const std::vector<POINT>& pts) const override {
        return {static_cast<int>(pts[0].x), static_cast<int>(pts[0].y)};
    }
};

struct Reg {
    Reg() {
        ShapeRenderer::registerDrawer<PointShape>([](HDC hdc, const Shape* s) {
            const PointShape* p = static_cast<const PointShape*>(s);
            Ellipse(hdc, p->left(), p->top(), p->right(), p->bottom());
        });
        ShapeFactory::registerCreator(std::make_unique<PointCreator>());
        MouseToolRegistry::registerTool(std::make_unique<PointMouse>());
    }
};
static Reg reg;

} // namespace
