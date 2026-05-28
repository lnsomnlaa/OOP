#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <istream>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include "../../HostServices.h"
#include "../../Shape.h"
#include "../../ShapeBinaryRegistry.h"
#include "../../ShapeFactory.h"
#include "../../ShapeTags.h"

namespace {

static HostServices* gHostServices = nullptr;

constexpr uint32_t kStarBinaryTag = ShapeTags::PluginTagMin + 1;

/**
 * Rhombus aligned to screen axes: center (cx,cy) and half-width / half-height (rw,rh).
 */
class StarShape final : public Shape {
public:
    StarShape(int cx, int cy, int rw, int rh) : cx_(cx), cy_(cy), rw_(rw), rh_(rh) {}

    uint32_t binaryTypeTag() const override { return kStarBinaryTag; }

    void setDiamond(int cx, int cy, int rw, int rh) {
        cx_ = cx;
        cy_ = cy;
        rw_ = rw < 1 ? 1 : rw;
        rh_ = rh < 1 ? 1 : rh;
    }

    int cx() const { return cx_; }
    int cy() const { return cy_; }
    int rw() const { return rw_; }
    int rh() const { return rh_; }

private:
    int cx_;
    int cy_;
    int rw_;
    int rh_;
};

void writeI32(std::ostream& os, int32_t v) {
    os.write(reinterpret_cast<const char*>(&v), sizeof(v));
}

bool readI32(std::istream& is, int32_t& v) {
    is.read(reinterpret_cast<char*>(&v), sizeof(v));
    return !is.fail();
}

/** Local copy of integer parsing so the DLL does not import symbols from the host EXE. */
bool parseIntsLocal(const std::string& text, std::vector<int>& out) {
    out.clear();
    std::istringstream iss(text);
    long v = 0;
    while (iss >> v) {
        out.push_back(static_cast<int>(v));
    }
    return !out.empty();
}

class StarCreator final : public IShapeCreator {
public:
    Shape* create(const std::vector<int>& p) const override {
        if (p.size() < 4) {
            return nullptr;
        }
        return new StarShape(p[0], p[1], p[2], p[3]);
    }
    size_t minParamCount() const override { return 4; }
    const char* commandId() const override { return "star"; }
};

class StarMouse final : public IMouseTool {
public:
    size_t pointCount() const override { return 2; }
    const char* commandId() const override { return "star"; }
    std::vector<int> toParams(const std::vector<POINT>& pts) const override {
        const int cx = static_cast<int>(pts[0].x);
        const int cy = static_cast<int>(pts[0].y);
        int rw = static_cast<int>(std::labs(static_cast<long>(pts[1].x - cx)));
        int rh = static_cast<int>(std::labs(static_cast<long>(pts[1].y - cy)));
        if (rw < 1) {
            rw = 1;
        }
        if (rh < 1) {
            rh = 1;
        }
        return {cx, cy, rw, rh};
    }
};

class StarCodec final : public IShapeBinaryCodec {
public:
    uint32_t tag() const override { return kStarBinaryTag; }
    const char* listLabel() const override { return "star"; }

    void writePayload(std::ostream& os, const Shape* shape) const override {
        const auto* s = static_cast<const StarShape*>(shape);
        writeI32(os, static_cast<int32_t>(s->cx()));
        writeI32(os, static_cast<int32_t>(s->cy()));
        writeI32(os, static_cast<int32_t>(s->rw()));
        writeI32(os, static_cast<int32_t>(s->rh()));
    }

    Shape* readPayload(std::istream& is) const override {
        int32_t cx = 0, cy = 0, rw = 0, rh = 0;
        if (!readI32(is, cx) || !readI32(is, cy) || !readI32(is, rw) || !readI32(is, rh)) {
            return nullptr;
        }
        return new StarShape(cx, cy, rw, rh);
    }

    bool editProperties(HWND owner, Shape* shape) const override {
        auto* s = static_cast<StarShape*>(shape);
        char buf[128] = {};
        std::snprintf(buf, sizeof(buf), "%d %d %d %d", s->cx(), s->cy(), s->rw(), s->rh());
        std::string txt = buf;
        if (!gHostServices ||
            !gHostServices->editParamsDialog(owner, "Edit star (cx cy halfWidth halfHeight)", txt)) {
            return false;
        }
        std::vector<int> p;
        if (!parseIntsLocal(txt, p) || p.size() != 4) {
            return false;
        }
        s->setDiamond(p[0], p[1], p[2], p[3]);
        return true;
    }
};

} // namespace

/**
 * DLL entry invoked by the host after LoadLibrary + GetProcAddress.
 * Registers factory, mouse tool, renderer, and binary codec for the new shape.
 */
extern "C" __declspec(dllexport) void __cdecl ShapesPlugin_OnLoad(HostServices* host) {
    if (!host) {
        return;
    }
    gHostServices = host;

    host->registerDrawerForTag(kStarBinaryTag, [](HDC hdc, const Shape* s) {
        const auto* st = static_cast<const StarShape*>(s);
        const int cx = st->cx(), cy = st->cy(), rw = st->rw(), rh = st->rh();
        MoveToEx(hdc, cx, cy - rh, nullptr);
        LineTo(hdc, cx + rw, cy);
        LineTo(hdc, cx, cy + rh);
        LineTo(hdc, cx - rw, cy);
        LineTo(hdc, cx, cy - rh);
    });

    host->registerCreator(std::make_unique<StarCreator>());
    host->registerTool(std::make_unique<StarMouse>());
    host->registerCodec(std::make_unique<StarCodec>());
}
