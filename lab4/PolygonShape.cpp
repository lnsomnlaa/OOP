#include "PolygonShape.h"
#include "ShapeFactory.h"
#include "ShapeRenderer.h"
#include "ShapeTags.h"
#include <memory>
#include <vector>
#include <windows.h>

PolygonShape::PolygonShape(std::vector<POINT> points) : pts(std::move(points)) {}

void PolygonShape::setPoints(std::vector<POINT> points) {
    pts = std::move(points);
}

namespace {

class PolygonCreator : public IShapeCreator {
public:
    Shape* create(const std::vector<int>& p) const override {
        if (p.size() < 6 || (p.size() % 2) != 0) {
            return nullptr;
        }
        std::vector<POINT> pts;
        pts.reserve(p.size() / 2);
        for (size_t i = 0; i + 1 < p.size(); i += 2) {
            POINT pt;
            pt.x = p[i];
            pt.y = p[i + 1];
            pts.push_back(pt);
        }
        return new PolygonShape(std::move(pts));
    }

    size_t minParamCount() const override { return 6; } 
    const char* commandId() const override { return "polygon"; }
};

class PolygonMouse : public IMouseTool {
public:
    size_t pointCount() const override {
        return MouseToolRegistry::pointCountOr(commandId(), 3);
    }
    const char* commandId() const override { return "polygon"; }
    std::vector<int> toParams(const std::vector<POINT>& pts) const override {
        std::vector<int> p;
        p.reserve(pts.size() * 2);
        for (const POINT& pt : pts) {
            p.push_back(static_cast<int>(pt.x));
            p.push_back(static_cast<int>(pt.y));
        }
        return p;
    }
};

struct Reg {
    Reg() {
        ShapeRenderer::registerDrawerForTag(ShapeTags::Polygon, [](HDC hdc, const Shape* s) {
            const PolygonShape* poly = static_cast<const PolygonShape*>(s);
            const auto& pts = poly->points();
            if (pts.size() < 2) {
                return;
            }
            if (pts.size() == 2) {
                MoveToEx(hdc, pts[0].x, pts[0].y, nullptr);
                LineTo(hdc, pts[1].x, pts[1].y);
                return;
            }
            Polygon(hdc, pts.data(), static_cast<int>(pts.size()));
        });
        ShapeFactory::registerCreator(std::make_unique<PolygonCreator>());
        MouseToolRegistry::registerTool(std::make_unique<PolygonMouse>());
    }
};
static Reg reg;

} // namespace

