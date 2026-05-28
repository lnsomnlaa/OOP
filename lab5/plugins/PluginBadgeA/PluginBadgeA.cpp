#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdint>
#include <cstdio>
#include <memory>
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
constexpr uint32_t kBadgeATag = ShapeTags::PluginTagMin + 101;

bool parseIntsLocal(const std::string& text, std::vector<int>& out) {
    out.clear();
    std::istringstream iss(text);
    long v = 0;
    while (iss >> v) {
        out.push_back(static_cast<int>(v));
    }
    return !out.empty();
}

class BadgeAShape final : public Shape {
public:
    BadgeAShape(int l, int t, int r, int b) : l_(l), t_(t), r_(r), b_(b) {}
    uint32_t binaryTypeTag() const override { return kBadgeATag; }
    int left() const { return l_; }
    int top() const { return t_; }
    int right() const { return r_; }
    int bottom() const { return b_; }
    void setBounds(int l, int t, int r, int b) {
        l_ = l;
        t_ = t;
        r_ = r;
        b_ = b;
    }

private:
    int l_;
    int t_;
    int r_;
    int b_;
};

void normalizeBounds(const BadgeAShape* b, int& l, int& t, int& r, int& bot) {
    l = b->left();
    t = b->top();
    r = b->right();
    bot = b->bottom();
    if (l > r) {
        const int tmp = l;
        l = r;
        r = tmp;
    }
    if (t > bot) {
        const int tmp = t;
        t = bot;
        bot = tmp;
    }
}

void writeI32(std::ostream& os, int32_t v) {
    os.write(reinterpret_cast<const char*>(&v), sizeof(v));
}

bool readI32(std::istream& is, int32_t& v) {
    is.read(reinterpret_cast<char*>(&v), sizeof(v));
    return !is.fail();
}

class BadgeACreator final : public IShapeCreator {
public:
    Shape* create(const std::vector<int>& p) const override {
        if (p.size() < 4) {
            return nullptr;
        }
        return new BadgeAShape(p[0], p[1], p[2], p[3]);
    }
    size_t minParamCount() const override { return 4; }
    const char* commandId() const override { return "badge_a"; }
};

class BadgeAMouse final : public IMouseTool {
public:
    size_t pointCount() const override { return 2; }
    const char* commandId() const override { return "badge_a"; }
    std::vector<int> toParams(const std::vector<POINT>& pts) const override {
        return {static_cast<int>(pts[0].x), static_cast<int>(pts[0].y), static_cast<int>(pts[1].x),
                static_cast<int>(pts[1].y)};
    }
};

class BadgeACodec final : public IShapeBinaryCodec {
public:
    uint32_t tag() const override { return kBadgeATag; }
    const char* listLabel() const override { return "badge_a [A]"; }
    void writePayload(std::ostream& os, const Shape* s) const override {
        const auto* b = static_cast<const BadgeAShape*>(s);
        writeI32(os, b->left());
        writeI32(os, b->top());
        writeI32(os, b->right());
        writeI32(os, b->bottom());
    }
    Shape* readPayload(std::istream& is) const override {
        int32_t l, t, r, b;
        if (!readI32(is, l) || !readI32(is, t) || !readI32(is, r) || !readI32(is, b)) {
            return nullptr;
        }
        return new BadgeAShape(l, t, r, b);
    }
    bool editProperties(HWND owner, Shape* s) const override {
        auto* b = static_cast<BadgeAShape*>(s);
        char buf[256];
        std::snprintf(buf, sizeof(buf), "%d %d %d %d", b->left(), b->top(), b->right(), b->bottom());
        std::string txt = buf;
        if (!gHostServices || !gHostServices->editParamsDialog(owner, "Edit badge_a (left top right bottom)", txt)) {
            return false;
        }
        std::vector<int> p;
        if (!parseIntsLocal(txt, p) || p.size() != 4) {
            return false;
        }
        b->setBounds(p[0], p[1], p[2], p[3]);
        return true;
    }
};

}

extern "C" __declspec(dllexport) void __cdecl ShapesPlugin_OnLoad(HostServices* host) {
    if (!host) {
        return;
    }
    gHostServices = host;
    host->registerDrawerForTag(kBadgeATag, [](HDC hdc, const Shape* s) {
        const auto* b = static_cast<const BadgeAShape*>(s);
        int l = 0, t = 0, r = 0, bot = 0;
        normalizeBounds(b, l, t, r, bot);
        const int w = r - l;
        const int h = bot - t;
        if (w < 8 || h < 8) {
            return;
        }
        const int cx = (l + r) / 2;
        const int cy = (t + bot) / 2;

        POINT star[10] = {
            {cx, t},
            {cx + w / 8, cy - h / 6},
            {r, cy},
            {cx + w / 8, cy + h / 6},
            {cx, bot},
            {cx - w / 8, cy + h / 6},
            {l, cy},
            {cx - w / 8, cy - h / 6},
            {cx, t},
            {cx + w / 8, cy - h / 6},
        };
        Polyline(hdc, star, 10);

        MoveToEx(hdc, l + w / 6, t + h / 6, nullptr);
        LineTo(hdc, r - w / 6, bot - h / 6);
        MoveToEx(hdc, l + w / 6, bot - h / 6, nullptr);
        LineTo(hdc, r - w / 6, t + h / 6);

        SetBkMode(hdc, TRANSPARENT);
    });
    host->registerCreator(std::make_unique<BadgeACreator>());
    host->registerTool(std::make_unique<BadgeAMouse>());
    host->registerCodec(std::make_unique<BadgeACodec>());
}
