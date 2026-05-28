#include "LineShape.h"
#include "ShapeFactory.h"
#include "ShapeRenderer.h"
#include "ShapeTags.h"
#include <memory>
#include <vector>
#include <windows.h>

LineShape::LineShape(int x1, int y1, int x2, int y2) : ax1(x1), ay1(y1), ax2(x2), ay2(y2) {}

void LineShape::setEndpoints(int nx1, int ny1, int nx2, int ny2) {
    ax1 = nx1;
    ay1 = ny1;
    ax2 = nx2;
    ay2 = ny2;
}

namespace {

class LineCreator : public IShapeCreator {
public:
    Shape* create(const std::vector<int>& p) const override {
        return new LineShape(p[0], p[1], p[2], p[3]);
    }
    size_t minParamCount() const override { return 4; }
    const char* commandId() const override { return "line"; }
};

class LineMouse : public IMouseTool {
public:
    size_t pointCount() const override { return 2; }
    const char* commandId() const override { return "line"; }
    std::vector<int> toParams(const std::vector<POINT>& pts) const override {
        return {static_cast<int>(pts[0].x), static_cast<int>(pts[0].y),
                static_cast<int>(pts[1].x), static_cast<int>(pts[1].y)};
    }
};

struct Reg {
    Reg() {
        ShapeRenderer::registerDrawerForTag(ShapeTags::Line, [](HDC hdc, const Shape* s) {
            const LineShape* L = static_cast<const LineShape*>(s);
            MoveToEx(hdc, L->x1(), L->y1(), nullptr);
            LineTo(hdc, L->x2(), L->y2());
        });
        ShapeFactory::registerCreator(std::make_unique<LineCreator>());
        MouseToolRegistry::registerTool(std::make_unique<LineMouse>());
    }
};
static Reg reg;

} // namespace
