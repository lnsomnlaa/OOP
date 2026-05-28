#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdint>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "../../HostServices.h"
#include "../../StorageProcessorRegistry.h"

namespace {

static HostServices* gHost = nullptr;

bool parseIntsLocal(const std::string& text, std::vector<int>& out) {
    out.clear();
    std::istringstream iss(text);
    long v = 0;
    while (iss >> v) {
        out.push_back(static_cast<int>(v));
    }
    return !out.empty();
}

class ArchiveRleProcessor final : public IStorageProcessor {
public:
    const char* id() const override { return "archive-rle"; }
    const char* displayName() const override { return "RLE archiver (plugin)"; }

    bool encode(const std::vector<uint8_t>& in, std::vector<uint8_t>& out, std::string& errorOut) const override {
        out.clear();
        out.reserve(in.size() + 32);
        out.push_back('R');
        out.push_back('L');
        out.push_back('E');
        out.push_back('1');
        writeU32(out, static_cast<uint32_t>(in.size()));

        size_t i = 0;
        while (i < in.size()) {
            size_t run = 1;
            while (i + run < in.size() && in[i + run] == in[i] && run < 128) {
                ++run;
            }
            if (run >= minRun_) {
                const uint8_t hdr = static_cast<uint8_t>(0x80u | static_cast<uint8_t>(run - 1));
                out.push_back(hdr);
                out.push_back(in[i]);
                i += run;
                continue;
            }

            const size_t litStart = i;
            size_t litLen = 0;
            while (i < in.size() && litLen < 128) {
                size_t lookRun = 1;
                while (i + lookRun < in.size() && in[i + lookRun] == in[i] && lookRun < 128) {
                    ++lookRun;
                }
                if (lookRun >= minRun_) {
                    break;
                }
                ++i;
                ++litLen;
            }
            if (litLen == 0) {
                ++i;
                litLen = 1;
            }
            out.push_back(static_cast<uint8_t>(litLen - 1));
            out.insert(out.end(), in.begin() + static_cast<std::ptrdiff_t>(litStart),
                       in.begin() + static_cast<std::ptrdiff_t>(litStart + litLen));
        }
        errorOut.clear();
        return true;
    }

    bool decode(const std::vector<uint8_t>& in, std::vector<uint8_t>& out, std::string& errorOut) const override {
        if (in.size() < 8 || in[0] != 'R' || in[1] != 'L' || in[2] != 'E' || in[3] != '1') {
            errorOut = "archive-rle: invalid header";
            return false;
        }
        size_t pos = 4;
        const uint32_t expected = readU32(in, pos);
        if (pos > in.size()) {
            errorOut = "archive-rle: truncated size header";
            return false;
        }

        out.clear();
        out.reserve(expected);
        while (pos < in.size()) {
            const uint8_t hdr = in[pos++];
            const size_t len = static_cast<size_t>((hdr & 0x7Fu) + 1u);
            if (hdr & 0x80u) {
                if (pos >= in.size()) {
                    errorOut = "archive-rle: truncated run token";
                    return false;
                }
                const uint8_t b = in[pos++];
                out.insert(out.end(), len, b);
            } else {
                if (pos + len > in.size()) {
                    errorOut = "archive-rle: truncated literal token";
                    return false;
                }
                out.insert(out.end(), in.begin() + static_cast<std::ptrdiff_t>(pos),
                           in.begin() + static_cast<std::ptrdiff_t>(pos + len));
                pos += len;
            }
        }

        if (out.size() != expected) {
            errorOut = "archive-rle: decompressed size mismatch";
            return false;
        }
        errorOut.clear();
        return true;
    }

    bool configure(HWND owner) override {
        if (!gHost) {
            return false;
        }
        std::string text = std::to_string(static_cast<int>(minRun_));
        if (!gHost->editParamsDialog(owner, "archive-rle min run (2..32)", text)) {
            return false;
        }
        std::vector<int> p;
        if (!parseIntsLocal(text, p) || p.size() != 1) {
            return false;
        }
        const int v = p[0] < 2 ? 2 : (p[0] > 32 ? 32 : p[0]);
        minRun_ = static_cast<size_t>(v);
        return true;
    }

private:
    static void writeU32(std::vector<uint8_t>& out, uint32_t v) {
        out.push_back(static_cast<uint8_t>(v & 0xFFu));
        out.push_back(static_cast<uint8_t>((v >> 8) & 0xFFu));
        out.push_back(static_cast<uint8_t>((v >> 16) & 0xFFu));
        out.push_back(static_cast<uint8_t>((v >> 24) & 0xFFu));
    }

    static uint32_t readU32(const std::vector<uint8_t>& in, size_t& pos) {
        if (pos + 4 > in.size()) {
            pos = in.size() + 1;
            return 0;
        }
        const uint32_t b0 = static_cast<uint32_t>(in[pos]);
        const uint32_t b1 = static_cast<uint32_t>(in[pos + 1]);
        const uint32_t b2 = static_cast<uint32_t>(in[pos + 2]);
        const uint32_t b3 = static_cast<uint32_t>(in[pos + 3]);
        pos += 4;
        return (b0) | (b1 << 8) | (b2 << 16) | (b3 << 24);
    }

    size_t minRun_ = 3;
};

}

extern "C" __declspec(dllexport) void __cdecl ShapesPlugin_OnLoad(HostServices* host) {
    if (!host) {
        return;
    }
    gHost = host;
    host->registerStorageProcessor(std::make_unique<ArchiveRleProcessor>());
}
