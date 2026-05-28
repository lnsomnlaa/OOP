#include "Tetra3DShape.h"
#include "ShapeFactory.h"
#include "ShapeRenderer.h"
#include "Solid3D.h"
#include <memory>
#include <vector>
#include <windows.h>

Tetra3DShape::Tetra3DShape(bool projected3d, int a, int b, int c, int d, int e, int f)
    : projected3d_(projected3d), ax1(0), ay1(0), ax2(0), ay2(0), ax3(0), ay3(0), CX(0), CY(0), CZ(0),
      edgeLen(0) {
    if (projected3d_) {
        CX = static_cast<double>(a);
        CY = static_cast<double>(b);
        CZ = static_cast<double>(c);
        edgeLen = static_cast<double>(d);
    } else {
        ax1 = a;
        ay1 = b;
        ax2 = c;
        ay2 = d;
        ax3 = e;
        ay3 = f;
    }
}

namespace {

class Tetra3DCreator : public IShapeCreator {
public:
    Shape* create(const std::vector<int>& p) const override {
        if (p.size() >= 6) {
            return new Tetra3DShape(false, p[0], p[1], p[2], p[3], p[4], p[5]);
        }
        if (p.size() >= 4) {
            return new Tetra3DShape(true, p[0], p[1], p[2], p[3], 0, 0);
        }
        return nullptr;
    }
    size_t minParamCount() const override { return 4; }
    const char* commandId() const override { return "tetra3d"; }
};

class Tetra3DMouse : public IMouseTool {
public:
    size_t pointCount() const override { return 3; }
    const char* commandId() const override { return "tetra3d"; }
    std::vector<int> toParams(const std::vector<POINT>& pts) const override {
        return {static_cast<int>(pts[0].x), static_cast<int>(pts[0].y),
                static_cast<int>(pts[1].x), static_cast<int>(pts[1].y),
                static_cast<int>(pts[2].x), static_cast<int>(pts[2].y)};
    }
};

struct Reg {
    Reg() {
        ShapeRenderer::registerDrawer<Tetra3DShape>([](HDC hdc, const Shape* s) {
            const Tetra3DShape* t = static_cast<const Tetra3DShape*>(s);
            if (!t->isProjected3D()) {
                POINT poly[3];
                poly[0].x = t->x1();
                poly[0].y = t->y1();
                poly[1].x = t->x2();
                poly[1].y = t->y2();
                poly[2].x = t->x3();
                poly[2].y = t->y3();
                Polygon(hdc, poly, 3);
                return;
            }
            TetraSolid3D solid(t->centerX(), t->centerY(), t->centerZ(), t->edge());
            for (const auto& seg : solid.projectedWireframe()) {
                MoveToEx(hdc, seg.first.x, seg.first.y, nullptr);
                LineTo(hdc, seg.second.x, seg.second.y);
            }
        });
        ShapeFactory::registerCreator(std::make_unique<Tetra3DCreator>());
        MouseToolRegistry::registerTool(std::make_unique<Tetra3DMouse>());
    }
};
static Reg reg;

} // namespace
