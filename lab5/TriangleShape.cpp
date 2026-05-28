#include "TriangleShape.h"
#include "ShapeFactory.h"
#include "ShapeRenderer.h"
#include "ShapeTags.h"
#include <memory>
#include <vector>
#include <windows.h>

TriangleShape::TriangleShape(int x1, int y1, int x2, int y2, int x3, int y3)
    : ax1(x1), ay1(y1), ax2(x2), ay2(y2), ax3(x3), ay3(y3) {}

void TriangleShape::setVertices(int x1, int y1, int x2, int y2, int x3, int y3) {
    ax1 = x1;
    ay1 = y1;
    ax2 = x2;
    ay2 = y2;
    ax3 = x3;
    ay3 = y3;
}

namespace {

class TriCreator : public IShapeCreator {
public:
    Shape* create(const std::vector<int>& p) const override {
        return new TriangleShape(p[0], p[1], p[2], p[3], p[4], p[5]);
    }
    size_t minParamCount() const override { return 6; }
    const char* commandId() const override { return "triangle"; }
};

class TriMouse : public IMouseTool {
public:
    size_t pointCount() const override { return 3; }
    const char* commandId() const override { return "triangle"; }
    std::vector<int> toParams(const std::vector<POINT>& pts) const override {
        return {static_cast<int>(pts[0].x), static_cast<int>(pts[0].y),
                static_cast<int>(pts[1].x), static_cast<int>(pts[1].y),
                static_cast<int>(pts[2].x), static_cast<int>(pts[2].y)};
    }
};

struct Reg {
    Reg() {
        ShapeRenderer::registerDrawerForTag(ShapeTags::Triangle, [](HDC hdc, const Shape* s) {
            const TriangleShape* t = static_cast<const TriangleShape*>(s);
            POINT poly[3];
            poly[0].x = t->x1();
            poly[0].y = t->y1();
            poly[1].x = t->x2();
            poly[1].y = t->y2();
            poly[2].x = t->x3();
            poly[2].y = t->y3();
            Polygon(hdc, poly, 3);
        });
        ShapeFactory::registerCreator(std::make_unique<TriCreator>());
        MouseToolRegistry::registerTool(std::make_unique<TriMouse>());
    }
};
static Reg reg;

} // namespace
