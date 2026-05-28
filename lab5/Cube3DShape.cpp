#include "Cube3DShape.h"
#include <algorithm>
#include "ShapeFactory.h"
#include "ShapeRenderer.h"
#include "ShapeTags.h"
#include "Solid3D.h"
#include <memory>
#include <vector>
#include <windows.h>

Cube3DShape::Cube3DShape(bool projected3d, int a, int b, int c, int d)
    : projected3d_(projected3d), L(0), T(0), R(0), B(0), CX(0), CY(0), CZ(0), edgeLen(0) {
    if (projected3d_) {
        CX = static_cast<double>(a);
        CY = static_cast<double>(b);
        CZ = static_cast<double>(c);
        edgeLen = static_cast<double>(d);
    } else {
        L = a;
        T = b;
        R = c;
        B = d;
    }
}

void Cube3DShape::setFlat(int left, int top, int right, int bottom) {
    projected3d_ = false;
    L = left;
    T = top;
    R = right;
    B = bottom;
}

void Cube3DShape::setProjected(int cx, int cy, int cz, int edge) {
    projected3d_ = true;
    CX = static_cast<double>(cx);
    CY = static_cast<double>(cy);
    CZ = static_cast<double>(cz);
    edgeLen = static_cast<double>(edge);
}

namespace {

class Cube3DCreator : public IShapeCreator {
public:
    Shape* create(const std::vector<int>& p) const override {
        if (p.size() < 4) {
            return nullptr;
        }
        const bool proj = p.size() > 4;
        return new Cube3DShape(proj, p[0], p[1], p[2], p[3]);
    }
    size_t minParamCount() const override { return 4; }
    const char* commandId() const override { return "cube3d"; }
};

class Cube3DMouse : public IMouseTool {
public:
    size_t pointCount() const override { return 2; }
    const char* commandId() const override { return "cube3d"; }
    std::vector<int> toParams(const std::vector<POINT>& pts) const override {
        const int x0 = static_cast<int>(pts[0].x), y0 = static_cast<int>(pts[0].y);
        const int x1 = static_cast<int>(pts[1].x), y1 = static_cast<int>(pts[1].y);
        return {(std::min)(x0, x1), (std::min)(y0, y1), (std::max)(x0, x1), (std::max)(y0, y1)};
    }
};

struct Reg {
    Reg() {
        ShapeRenderer::registerDrawerForTag(ShapeTags::Cube3D, [](HDC hdc, const Shape* s) {
            const Cube3DShape* c = static_cast<const Cube3DShape*>(s);
            if (!c->isProjected3D()) {
                Rectangle(hdc, c->left(), c->top(), c->right(), c->bottom());
                return;
            }
            CubeSolid3D solid(c->centerX(), c->centerY(), c->centerZ(), c->edge());
            for (const auto& seg : solid.projectedWireframe()) {
                MoveToEx(hdc, seg.first.x, seg.first.y, nullptr);
                LineTo(hdc, seg.second.x, seg.second.y);
            }
        });
        ShapeFactory::registerCreator(std::make_unique<Cube3DCreator>());
        MouseToolRegistry::registerTool(std::make_unique<Cube3DMouse>());
    }
};
static Reg reg;

} // namespace
