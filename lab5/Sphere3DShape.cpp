#include "Sphere3DShape.h"
#include "ShapeFactory.h"
#include "ShapeRenderer.h"
#include "ShapeTags.h"
#include "Solid3D.h"
#include <cmath>
#include <memory>
#include <vector>
#include <windows.h>

Sphere3DShape::Sphere3DShape(bool projected3d, int a, int b, int c, int d)
    : projected3d_(projected3d), scx(0), scy(0), sr(0), CX(0), CY(0), CZ(0), rad(0) {
    if (projected3d_) {
        CX = static_cast<double>(a);
        CY = static_cast<double>(b);
        CZ = static_cast<double>(c);
        rad = static_cast<double>(d);
    } else {
        scx = a;
        scy = b;
        sr = c;
    }
}

void Sphere3DShape::setFlat(int cx, int cy, int r) {
    projected3d_ = false;
    scx = cx;
    scy = cy;
    sr = r;
}

void Sphere3DShape::setProjected(int cx, int cy, int cz, int r) {
    projected3d_ = true;
    CX = static_cast<double>(cx);
    CY = static_cast<double>(cy);
    CZ = static_cast<double>(cz);
    rad = static_cast<double>(r);
}

namespace {

class Sphere3DCreator : public IShapeCreator {
public:
    Shape* create(const std::vector<int>& p) const override {
        if (p.size() < 3) {
            return nullptr;
        }
        if (p.size() > 3) {
            if (p.size() < 4) {
                return nullptr;
            }
            return new Sphere3DShape(true, p[0], p[1], p[2], p[3]);
        }
        return new Sphere3DShape(false, p[0], p[1], p[2], 0);
    }
    size_t minParamCount() const override { return 3; }
    const char* commandId() const override { return "sphere3d"; }
};

class Sphere3DMouse : public IMouseTool {
public:
    size_t pointCount() const override { return 2; }
    const char* commandId() const override { return "sphere3d"; }
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
        ShapeRenderer::registerDrawerForTag(ShapeTags::Sphere3D, [](HDC hdc, const Shape* s) {
            const Sphere3DShape* sp = static_cast<const Sphere3DShape*>(s);
            if (!sp->isProjected3D()) {
                const int cx = sp->scrCx(), cy = sp->scrCy(), r = sp->scrR();
                Ellipse(hdc, cx - r, cy - r, cx + r, cy + r);
                return;
            }
            SphereSolid3D solid(sp->centerX(), sp->centerY(), sp->centerZ(), sp->radius());
            for (const auto& seg : solid.projectedWireframe()) {
                MoveToEx(hdc, seg.first.x, seg.first.y, nullptr);
                LineTo(hdc, seg.second.x, seg.second.y);
            }
        });
        ShapeFactory::registerCreator(std::make_unique<Sphere3DCreator>());
        MouseToolRegistry::registerTool(std::make_unique<Sphere3DMouse>());
    }
};
static Reg reg;

} // namespace
