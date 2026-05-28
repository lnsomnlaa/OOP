/**
 * Binary codecs + property editors for all shape types.
 * Each concrete codec registers itself — no central switch/if chain on shape type.
 */
#include "ShapeBinaryRegistry.h"
#include "Lab3Dialogs.h"
#include "ShapeTags.h"

#include "CircleShape.h"
#include "Cube3DShape.h"
#include "EllipseShape.h"
#include "LineShape.h"
#include "PointShape.h"
#include "PolygonShape.h"
#include "RectangleShape.h"
#include "Sphere3DShape.h"
#include "Tetra3DShape.h"
#include "TriangleShape.h"

#include <cstdint>
#include <cstdio>
#include <memory>
#include <sstream>
#include <vector>

namespace {

void writeI32(std::ostream& os, int32_t v) {
    os.write(reinterpret_cast<const char*>(&v), sizeof(v));
}

bool readI32(std::istream& is, int32_t& v) {
    is.read(reinterpret_cast<char*>(&v), sizeof(v));
    return !is.fail();
}

void writeU8(std::ostream& os, uint8_t v) {
    os.write(reinterpret_cast<const char*>(&v), 1);
}

bool readU8(std::istream& is, uint8_t& v) {
    is.read(reinterpret_cast<char*>(&v), 1);
    return !is.fail();
}

void writeU32(std::ostream& os, uint32_t v) {
    os.write(reinterpret_cast<const char*>(&v), sizeof(v));
}

bool readU32(std::istream& is, uint32_t& v) {
    is.read(reinterpret_cast<char*>(&v), sizeof(v));
    return !is.fail();
}

void writeF64(std::ostream& os, double v) {
    os.write(reinterpret_cast<const char*>(&v), sizeof(v));
}

bool readF64(std::istream& is, double& v) {
    is.read(reinterpret_cast<char*>(&v), sizeof(v));
    return !is.fail();
}

struct LineCodec final : public IShapeBinaryCodec {
    uint32_t tag() const override { return ShapeTags::Line; }
    const char* listLabel() const override { return "line"; }
    void writePayload(std::ostream& os, const Shape* s) const override {
        const auto* L = static_cast<const LineShape*>(s);
        writeI32(os, L->x1());
        writeI32(os, L->y1());
        writeI32(os, L->x2());
        writeI32(os, L->y2());
    }
    Shape* readPayload(std::istream& is) const override {
        int32_t a, b, c, d;
        if (!readI32(is, a) || !readI32(is, b) || !readI32(is, c) || !readI32(is, d)) {
            return nullptr;
        }
        return new LineShape(a, b, c, d);
    }
    bool editProperties(HWND owner, Shape* s) const override {
        auto* L = static_cast<LineShape*>(s);
        char buf[256];
        snprintf(buf, sizeof(buf), "%d %d %d %d", L->x1(), L->y1(), L->x2(), L->y2());
        std::string txt = buf;
        if (!Lab3_EditTextDialog(owner, "Edit line (x1 y1 x2 y2)", txt)) {
            return false;
        }
        std::vector<int> p;
        if (!Lab3_ParseInts(txt, p) || p.size() != 4) {
            return false;
        }
        L->setEndpoints(p[0], p[1], p[2], p[3]);
        return true;
    }
};

struct RectCodec final : public IShapeBinaryCodec {
    uint32_t tag() const override { return ShapeTags::Rect; }
    const char* listLabel() const override { return "rect"; }
    void writePayload(std::ostream& os, const Shape* s) const override {
        const auto* r = static_cast<const RectangleShape*>(s);
        writeI32(os, r->left());
        writeI32(os, r->top());
        writeI32(os, r->right());
        writeI32(os, r->bottom());
    }
    Shape* readPayload(std::istream& is) const override {
        int32_t l, t, r, b;
        if (!readI32(is, l) || !readI32(is, t) || !readI32(is, r) || !readI32(is, b)) {
            return nullptr;
        }
        return new RectangleShape(l, t, r, b);
    }
    bool editProperties(HWND owner, Shape* s) const override {
        auto* r = static_cast<RectangleShape*>(s);
        char buf[256];
        snprintf(buf, sizeof(buf), "%d %d %d %d", r->left(), r->top(), r->right(), r->bottom());
        std::string txt = buf;
        if (!Lab3_EditTextDialog(owner, "Edit rect (left top right bottom)", txt)) {
            return false;
        }
        std::vector<int> p;
        if (!Lab3_ParseInts(txt, p) || p.size() != 4) {
            return false;
        }
        r->setBounds(p[0], p[1], p[2], p[3]);
        return true;
    }
};

struct EllipseCodec final : public IShapeBinaryCodec {
    uint32_t tag() const override { return ShapeTags::Ellipse; }
    const char* listLabel() const override { return "ellipse"; }
    void writePayload(std::ostream& os, const Shape* s) const override {
        const auto* e = static_cast<const EllipseShape*>(s);
        writeI32(os, e->left());
        writeI32(os, e->top());
        writeI32(os, e->right());
        writeI32(os, e->bottom());
    }
    Shape* readPayload(std::istream& is) const override {
        int32_t l, t, r, b;
        if (!readI32(is, l) || !readI32(is, t) || !readI32(is, r) || !readI32(is, b)) {
            return nullptr;
        }
        return new EllipseShape(l, t, r, b);
    }
    bool editProperties(HWND owner, Shape* s) const override {
        auto* e = static_cast<EllipseShape*>(s);
        char buf[256];
        snprintf(buf, sizeof(buf), "%d %d %d %d", e->left(), e->top(), e->right(), e->bottom());
        std::string txt = buf;
        if (!Lab3_EditTextDialog(owner, "Edit ellipse (left top right bottom)", txt)) {
            return false;
        }
        std::vector<int> p;
        if (!Lab3_ParseInts(txt, p) || p.size() != 4) {
            return false;
        }
        e->setBounds(p[0], p[1], p[2], p[3]);
        return true;
    }
};

struct CircleCodec final : public IShapeBinaryCodec {
    uint32_t tag() const override { return ShapeTags::Circle; }
    const char* listLabel() const override { return "circle"; }
    void writePayload(std::ostream& os, const Shape* s) const override {
        const auto* c = static_cast<const CircleShape*>(s);
        const int cx = (c->left() + c->right()) / 2;
        const int cy = (c->top() + c->bottom()) / 2;
        const int r = (c->right() - c->left()) / 2;
        writeI32(os, cx);
        writeI32(os, cy);
        writeI32(os, r);
    }
    Shape* readPayload(std::istream& is) const override {
        int32_t cx, cy, r;
        if (!readI32(is, cx) || !readI32(is, cy) || !readI32(is, r)) {
            return nullptr;
        }
        return new CircleShape(cx, cy, r);
    }
    bool editProperties(HWND owner, Shape* s) const override {
        auto* c = static_cast<CircleShape*>(s);
        const int cx = (c->left() + c->right()) / 2;
        const int cy = (c->top() + c->bottom()) / 2;
        const int r = (c->right() - c->left()) / 2;
        char buf[256];
        snprintf(buf, sizeof(buf), "%d %d %d", cx, cy, r);
        std::string txt = buf;
        if (!Lab3_EditTextDialog(owner, "Edit circle (cx cy r)", txt)) {
            return false;
        }
        std::vector<int> p;
        if (!Lab3_ParseInts(txt, p) || p.size() != 3) {
            return false;
        }
        c->setBounds(p[0] - p[2], p[1] - p[2], p[0] + p[2], p[1] + p[2]);
        return true;
    }
};

struct TriangleCodec final : public IShapeBinaryCodec {
    uint32_t tag() const override { return ShapeTags::Triangle; }
    const char* listLabel() const override { return "triangle"; }
    void writePayload(std::ostream& os, const Shape* s) const override {
        const auto* t = static_cast<const TriangleShape*>(s);
        writeI32(os, t->x1());
        writeI32(os, t->y1());
        writeI32(os, t->x2());
        writeI32(os, t->y2());
        writeI32(os, t->x3());
        writeI32(os, t->y3());
    }
    Shape* readPayload(std::istream& is) const override {
        int32_t x1, y1, x2, y2, x3, y3;
        if (!readI32(is, x1) || !readI32(is, y1) || !readI32(is, x2) || !readI32(is, y2) || !readI32(is, x3) ||
            !readI32(is, y3)) {
            return nullptr;
        }
        return new TriangleShape(x1, y1, x2, y2, x3, y3);
    }
    bool editProperties(HWND owner, Shape* s) const override {
        auto* t = static_cast<TriangleShape*>(s);
        char buf[256];
        snprintf(buf, sizeof(buf), "%d %d %d %d %d %d", t->x1(), t->y1(), t->x2(), t->y2(), t->x3(), t->y3());
        std::string txt = buf;
        if (!Lab3_EditTextDialog(owner, "Edit triangle (x1 y1 x2 y2 x3 y3)", txt)) {
            return false;
        }
        std::vector<int> p;
        if (!Lab3_ParseInts(txt, p) || p.size() != 6) {
            return false;
        }
        t->setVertices(p[0], p[1], p[2], p[3], p[4], p[5]);
        return true;
    }
};

struct PointCodec final : public IShapeBinaryCodec {
    uint32_t tag() const override { return ShapeTags::Point; }
    const char* listLabel() const override { return "point"; }
    void writePayload(std::ostream& os, const Shape* s) const override {
        const auto* p = static_cast<const PointShape*>(s);
        const int cx = (p->left() + p->right()) / 2;
        const int cy = (p->top() + p->bottom()) / 2;
        writeI32(os, cx);
        writeI32(os, cy);
    }
    Shape* readPayload(std::istream& is) const override {
        int32_t x, y;
        if (!readI32(is, x) || !readI32(is, y)) {
            return nullptr;
        }
        return new PointShape(x, y);
    }
    bool editProperties(HWND owner, Shape* s) const override {
        auto* p = static_cast<PointShape*>(s);
        const int cx = (p->left() + p->right()) / 2;
        const int cy = (p->top() + p->bottom()) / 2;
        char buf[128];
        snprintf(buf, sizeof(buf), "%d %d", cx, cy);
        std::string txt = buf;
        if (!Lab3_EditTextDialog(owner, "Edit point (x y)", txt)) {
            return false;
        }
        std::vector<int> v;
        if (!Lab3_ParseInts(txt, v) || v.size() != 2) {
            return false;
        }
        p->setCenter(v[0], v[1]);
        return true;
    }
};

struct PolygonCodec final : public IShapeBinaryCodec {
    uint32_t tag() const override { return ShapeTags::Polygon; }
    const char* listLabel() const override { return "polygon"; }
    void writePayload(std::ostream& os, const Shape* s) const override {
        const auto* p = static_cast<const PolygonShape*>(s);
        const uint32_t n = static_cast<uint32_t>(p->points().size());
        writeU32(os, n);
        for (const POINT& pt : p->points()) {
            writeI32(os, pt.x);
            writeI32(os, pt.y);
        }
    }
    Shape* readPayload(std::istream& is) const override {
        uint32_t n = 0;
        if (!readU32(is, n) || n < 2 || n > 4096) {
            return nullptr;
        }
        std::vector<POINT> pts;
        pts.reserve(n);
        for (uint32_t i = 0; i < n; ++i) {
            int32_t x = 0, y = 0;
            if (!readI32(is, x) || !readI32(is, y)) {
                return nullptr;
            }
            POINT pt{static_cast<LONG>(x), static_cast<LONG>(y)};
            pts.push_back(pt);
        }
        return new PolygonShape(std::move(pts));
    }
    bool editProperties(HWND owner, Shape* s) const override {
        auto* p = static_cast<PolygonShape*>(s);
        std::ostringstream oss;
        for (const POINT& pt : p->points()) {
            oss << pt.x << ' ' << pt.y << ' ';
        }
        std::string txt = oss.str();
        if (!Lab3_EditTextDialog(owner, "Edit polygon (x1 y1 x2 y2 ...)", txt)) {
            return false;
        }
        std::vector<int> v;
        if (!Lab3_ParseInts(txt, v) || v.size() < 4 || (v.size() % 2) != 0) {
            return false;
        }
        std::vector<POINT> pts;
        pts.reserve(v.size() / 2);
        for (size_t i = 0; i + 1 < v.size(); i += 2) {
            pts.push_back(POINT{static_cast<LONG>(v[i]), static_cast<LONG>(v[i + 1])});
        }
        p->setPoints(std::move(pts));
        return true;
    }
};

struct CubeCodec final : public IShapeBinaryCodec {
    uint32_t tag() const override { return ShapeTags::Cube3D; }
    const char* listLabel() const override { return "cube3d"; }
    void writePayload(std::ostream& os, const Shape* s) const override {
        const auto* c = static_cast<const Cube3DShape*>(s);
        writeU8(os, c->isProjected3D() ? 1u : 0u);
        if (c->isProjected3D()) {
            writeF64(os, c->centerX());
            writeF64(os, c->centerY());
            writeF64(os, c->centerZ());
            writeF64(os, c->edge());
        } else {
            writeI32(os, c->left());
            writeI32(os, c->top());
            writeI32(os, c->right());
            writeI32(os, c->bottom());
        }
    }
    Shape* readPayload(std::istream& is) const override {
        uint8_t proj = 0;
        if (!readU8(is, proj)) {
            return nullptr;
        }
        if (proj) {
            double cx, cy, cz, e;
            if (!readF64(is, cx) || !readF64(is, cy) || !readF64(is, cz) || !readF64(is, e)) {
                return nullptr;
            }
            return new Cube3DShape(true, static_cast<int>(cx + 0.5), static_cast<int>(cy + 0.5),
                                    static_cast<int>(cz + 0.5), static_cast<int>(e + 0.5));
        }
        int32_t l, t, r, b;
        if (!readI32(is, l) || !readI32(is, t) || !readI32(is, r) || !readI32(is, b)) {
            return nullptr;
        }
        return new Cube3DShape(false, l, t, r, b);
    }
    bool editProperties(HWND owner, Shape* s) const override {
        auto* c = static_cast<Cube3DShape*>(s);
        std::string txt;
        if (c->isProjected3D()) {
            char buf[256];
            snprintf(buf, sizeof(buf), "%d %d %d %d", static_cast<int>(c->centerX() + 0.5),
                     static_cast<int>(c->centerY() + 0.5), static_cast<int>(c->centerZ() + 0.5),
                     static_cast<int>(c->edge() + 0.5));
            txt = buf;
            if (!Lab3_EditTextDialog(owner, "Edit cube3d wire (cx cy cz edge)", txt)) {
                return false;
            }
            std::vector<int> p;
            if (!Lab3_ParseInts(txt, p) || p.size() != 4) {
                return false;
            }
            c->setProjected(p[0], p[1], p[2], p[3]);
            return true;
        }
        char buf[256];
        snprintf(buf, sizeof(buf), "%d %d %d %d", c->left(), c->top(), c->right(), c->bottom());
        txt = buf;
        if (!Lab3_EditTextDialog(owner, "Edit cube3d flat (L T R B)", txt)) {
            return false;
        }
        std::vector<int> p;
        if (!Lab3_ParseInts(txt, p) || p.size() != 4) {
            return false;
        }
        c->setFlat(p[0], p[1], p[2], p[3]);
        return true;
    }
};

struct SphereCodec final : public IShapeBinaryCodec {
    uint32_t tag() const override { return ShapeTags::Sphere3D; }
    const char* listLabel() const override { return "sphere3d"; }
    void writePayload(std::ostream& os, const Shape* s) const override {
        const auto* sp = static_cast<const Sphere3DShape*>(s);
        writeU8(os, sp->isProjected3D() ? 1u : 0u);
        if (sp->isProjected3D()) {
            writeF64(os, sp->centerX());
            writeF64(os, sp->centerY());
            writeF64(os, sp->centerZ());
            writeF64(os, sp->radius());
        } else {
            writeI32(os, sp->scrCx());
            writeI32(os, sp->scrCy());
            writeI32(os, sp->scrR());
        }
    }
    Shape* readPayload(std::istream& is) const override {
        uint8_t proj = 0;
        if (!readU8(is, proj)) {
            return nullptr;
        }
        if (proj) {
            double cx, cy, cz, r;
            if (!readF64(is, cx) || !readF64(is, cy) || !readF64(is, cz) || !readF64(is, r)) {
                return nullptr;
            }
            return new Sphere3DShape(true, static_cast<int>(cx + 0.5), static_cast<int>(cy + 0.5),
                                     static_cast<int>(cz + 0.5), static_cast<int>(r + 0.5));
        }
        int32_t cx, cy, r;
        if (!readI32(is, cx) || !readI32(is, cy) || !readI32(is, r)) {
            return nullptr;
        }
        return new Sphere3DShape(false, cx, cy, r, 0);
    }
    bool editProperties(HWND owner, Shape* s) const override {
        auto* sp = static_cast<Sphere3DShape*>(s);
        std::string txt;
        if (sp->isProjected3D()) {
            char buf[256];
            snprintf(buf, sizeof(buf), "%d %d %d %d", static_cast<int>(sp->centerX() + 0.5),
                     static_cast<int>(sp->centerY() + 0.5), static_cast<int>(sp->centerZ() + 0.5),
                     static_cast<int>(sp->radius() + 0.5));
            txt = buf;
            if (!Lab3_EditTextDialog(owner, "Edit sphere3d wire (cx cy cz r)", txt)) {
                return false;
            }
            std::vector<int> p;
            if (!Lab3_ParseInts(txt, p) || p.size() != 4) {
                return false;
            }
            sp->setProjected(p[0], p[1], p[2], p[3]);
            return true;
        }
        char buf[256];
        snprintf(buf, sizeof(buf), "%d %d %d", sp->scrCx(), sp->scrCy(), sp->scrR());
        txt = buf;
        if (!Lab3_EditTextDialog(owner, "Edit sphere3d flat (cx cy r)", txt)) {
            return false;
        }
        std::vector<int> p;
        if (!Lab3_ParseInts(txt, p) || p.size() != 3) {
            return false;
        }
        sp->setFlat(p[0], p[1], p[2]);
        return true;
    }
};

struct TetraCodec final : public IShapeBinaryCodec {
    uint32_t tag() const override { return ShapeTags::Tetra3D; }
    const char* listLabel() const override { return "tetra3d"; }
    void writePayload(std::ostream& os, const Shape* s) const override {
        const auto* t = static_cast<const Tetra3DShape*>(s);
        writeU8(os, t->isProjected3D() ? 1u : 0u);
        if (t->isProjected3D()) {
            writeF64(os, t->centerX());
            writeF64(os, t->centerY());
            writeF64(os, t->centerZ());
            writeF64(os, t->edge());
        } else {
            writeI32(os, t->x1());
            writeI32(os, t->y1());
            writeI32(os, t->x2());
            writeI32(os, t->y2());
            writeI32(os, t->x3());
            writeI32(os, t->y3());
        }
    }
    Shape* readPayload(std::istream& is) const override {
        uint8_t proj = 0;
        if (!readU8(is, proj)) {
            return nullptr;
        }
        if (proj) {
            double cx, cy, cz, e;
            if (!readF64(is, cx) || !readF64(is, cy) || !readF64(is, cz) || !readF64(is, e)) {
                return nullptr;
            }
            return new Tetra3DShape(true, static_cast<int>(cx + 0.5), static_cast<int>(cy + 0.5),
                                    static_cast<int>(cz + 0.5), static_cast<int>(e + 0.5), 0, 0);
        }
        int32_t x1, y1, x2, y2, x3, y3;
        if (!readI32(is, x1) || !readI32(is, y1) || !readI32(is, x2) || !readI32(is, y2) || !readI32(is, x3) ||
            !readI32(is, y3)) {
            return nullptr;
        }
        return new Tetra3DShape(false, x1, y1, x2, y2, x3, y3);
    }
    bool editProperties(HWND owner, Shape* s) const override {
        auto* t = static_cast<Tetra3DShape*>(s);
        std::string txt;
        if (t->isProjected3D()) {
            char buf[256];
            snprintf(buf, sizeof(buf), "%d %d %d %d", static_cast<int>(t->centerX() + 0.5),
                     static_cast<int>(t->centerY() + 0.5), static_cast<int>(t->centerZ() + 0.5),
                     static_cast<int>(t->edge() + 0.5));
            txt = buf;
            if (!Lab3_EditTextDialog(owner, "Edit tetra3d wire (cx cy cz edge)", txt)) {
                return false;
            }
            std::vector<int> p;
            if (!Lab3_ParseInts(txt, p) || p.size() != 4) {
                return false;
            }
            t->setProjected(p[0], p[1], p[2], p[3]);
            return true;
        }
        char buf[256];
        snprintf(buf, sizeof(buf), "%d %d %d %d %d %d", t->x1(), t->y1(), t->x2(), t->y2(), t->x3(), t->y3());
        txt = buf;
        if (!Lab3_EditTextDialog(owner, "Edit tetra3d flat (x1 y1 x2 y2 x3 y3)", txt)) {
            return false;
        }
        std::vector<int> p;
        if (!Lab3_ParseInts(txt, p) || p.size() != 6) {
            return false;
        }
        t->setFlat(p[0], p[1], p[2], p[3], p[4], p[5]);
        return true;
    }
};

struct RegAll {
    RegAll() {
        ShapeBinaryRegistry::registerCodec(std::make_unique<LineCodec>());
        ShapeBinaryRegistry::registerCodec(std::make_unique<RectCodec>());
        ShapeBinaryRegistry::registerCodec(std::make_unique<EllipseCodec>());
        ShapeBinaryRegistry::registerCodec(std::make_unique<CircleCodec>());
        ShapeBinaryRegistry::registerCodec(std::make_unique<TriangleCodec>());
        ShapeBinaryRegistry::registerCodec(std::make_unique<PointCodec>());
        ShapeBinaryRegistry::registerCodec(std::make_unique<PolygonCodec>());
        ShapeBinaryRegistry::registerCodec(std::make_unique<CubeCodec>());
        ShapeBinaryRegistry::registerCodec(std::make_unique<SphereCodec>());
        ShapeBinaryRegistry::registerCodec(std::make_unique<TetraCodec>());
    }
};
static RegAll s_regAll;

} // namespace
