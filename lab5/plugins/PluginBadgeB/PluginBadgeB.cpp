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
constexpr uint32_t kBadgeBTag = ShapeTags::PluginTagMin + 102;

bool parseIntsLocal(const std::string& text, std::vector<int>& out) {
    out.clear();
    std::istringstream iss(text);
    long v = 0;
    while (iss >> v) {
        out.push_back(static_cast<int>(v));
    }
    return !out.empty();
}

class BadgeBShape final : public Shape {
public:
    BadgeBShape(int l, int t, int r, int b) : l_(l), t_(t), r_(r), b_(b) {}
    uint32_t binaryTypeTag() const override { return kBadgeBTag; }
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

void normalizeBounds(const BadgeBShape* b, int& l, int& t, int& r, int& bot) {
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

class BadgeBCreator final : public IShapeCreator {
public:
    Shape* create(const std::vector<int>& p) const override {
        if (p.size() < 4) {
            return nullptr;
        }
        return new BadgeBShape(p[0], p[1], p[2], p[3]);
    }
    size_t minParamCount() const override { return 4; }
    const char* commandId() const override { return "badge_b"; }
};

class BadgeBMouse final : public IMouseTool {
public:
    size_t pointCount() const override { return 2; }
    const char* commandId() const override { return "badge_b"; }
    std::vector<int> toParams(const std::vector<POINT>& pts) const override {
        return {static_cast<int>(pts[0].x), static_cast<int>(pts[0].y), static_cast<int>(pts[1].x),
                static_cast<int>(pts[1].y)};
    }
};

class BadgeBCodec final : public IShapeBinaryCodec {
public:
    uint32_t tag() const override { return kBadgeBTag; }
    const char* listLabel() const override { return "badge_b [B]"; }
    void writePayload(std::ostream& os, const Shape* s) const override {
        const auto* b = static_cast<const BadgeBShape*>(s);
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
        return new BadgeBShape(l, t, r, b);
    }
    bool editProperties(HWND owner, Shape* s) const override {
        auto* b = static_cast<BadgeBShape*>(s);
        char buf[256];
        std::snprintf(buf, sizeof(buf), "%d %d %d %d", b->left(), b->top(), b->right(), b->bottom());
        std::string txt = buf;
        if (!gHostServices || !gHostServices->editParamsDialog(owner, "Edit badge_b (left top right bottom)", txt)) {
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
    host->registerDrawerForTag(kBadgeBTag, [](HDC hdc, const Shape* s) {
        const auto* b = static_cast<const BadgeBShape*>(s);
        int l = 0, t = 0, r = 0, bot = 0;
        normalizeBounds(b, l, t, r, bot);
        const int w = r - l;
        const int h = bot - t;
        if (w < 8 || h < 8) {
            return;
        }
        const int cx = (l + r) / 2;
        const int cy = (t + bot) / 2;

        POINT hex[7] = {
            {l + w / 4, t},
            {l + (3 * w) / 4, t},
            {r, cy},
            {l + (3 * w) / 4, bot},
            {l + w / 4, bot},
            {l, cy},
            {l + w / 4, t},
        };
        Polyline(hdc, hex, 7);

        POINT bolt[5] = {
            {cx + w / 10, t + h / 5},
            {cx - w / 16, cy - h / 14},
            {cx + w / 16, cy - h / 14},
            {cx - w / 10, bot - h / 5},
            {cx + w / 14, cy + h / 14},
        };
        Polyline(hdc, bolt, 5);

        SetBkMode(hdc, TRANSPARENT);
    });
    host->registerCreator(std::make_unique<BadgeBCreator>());
    host->registerTool(std::make_unique<BadgeBMouse>());
    host->registerCodec(std::make_unique<BadgeBCodec>());
}
