#include "ShapeBinaryRegistry.h"
#include "Shape.h"
#include "ShapeList.h"
#include "StorageProcessorRegistry.h"

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>

namespace {

std::unordered_map<uint32_t, std::unique_ptr<IShapeBinaryCodec>>& codecs() {
    static std::unordered_map<uint32_t, std::unique_ptr<IShapeBinaryCodec>> m;
    return m;
}

void writeU32(std::ostream& os, uint32_t v) {
    os.write(reinterpret_cast<const char*>(&v), sizeof(v));
}

bool readU32(std::istream& is, uint32_t& v) {
    is.read(reinterpret_cast<char*>(&v), sizeof(v));
    return !is.fail();
}

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

void writeBytes(std::ostream& os, const std::vector<uint8_t>& data) {
    if (!data.empty()) {
        os.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
    }
}

bool readBytes(std::istream& is, std::vector<uint8_t>& out, size_t n) {
    out.resize(n);
    if (n == 0) {
        return true;
    }
    is.read(reinterpret_cast<char*>(out.data()), static_cast<std::streamsize>(n));
    return !is.fail();
}

void writeString(std::ostream& os, const std::string& s) {
    writeU32(os, static_cast<uint32_t>(s.size()));
    if (!s.empty()) {
        os.write(s.data(), static_cast<std::streamsize>(s.size()));
    }
}

bool readString(std::istream& is, std::string& s) {
    uint32_t n = 0;
    if (!readU32(is, n) || n > 1024) {
        return false;
    }
    s.resize(n);
    if (n == 0) {
        return true;
    }
    is.read(&s[0], static_cast<std::streamsize>(n));
    return !is.fail();
}

constexpr uint32_t kRawMagic = 0x3342414Cu;
constexpr uint32_t kRawVersion = 1u;

bool serializeRawList(const ShapeList& list, std::vector<uint8_t>& out) {
    std::ostringstream os(std::ios::binary);
    writeU32(os, kRawMagic);
    writeU32(os, kRawVersion);
    const uint32_t n = static_cast<uint32_t>(list.count());
    writeU32(os, n);
    for (uint32_t i = 0; i < n; ++i) {
        const Shape* s = list.at(i);
        if (!s) {
            return false;
        }
        const uint32_t tag = s->binaryTypeTag();
        const IShapeBinaryCodec* codec = ShapeBinaryRegistry::find(tag);
        if (!codec) {
            return false;
        }
        writeU32(os, tag);
        codec->writePayload(os, s);
    }
    const std::string tmp = os.str();
    out.assign(tmp.begin(), tmp.end());
    return true;
}

bool deserializeRawList(ShapeList& list, const std::vector<uint8_t>& data) {
    std::string tmp(data.begin(), data.end());
    std::istringstream is(tmp, std::ios::binary);
    uint32_t magic = 0;
    uint32_t ver = 0;
    uint32_t n = 0;
    if (!readU32(is, magic) || magic != kRawMagic) {
        return false;
    }
    if (!readU32(is, ver) || ver != kRawVersion) {
        return false;
    }
    if (!readU32(is, n)) {
        return false;
    }
    std::vector<Shape*> loaded;
    loaded.reserve(n);
    for (uint32_t i = 0; i < n; ++i) {
        uint32_t tag = 0;
        if (!readU32(is, tag)) {
            for (Shape* s : loaded) {
                delete s;
            }
            return false;
        }
        const IShapeBinaryCodec* codec = ShapeBinaryRegistry::find(tag);
        if (!codec) {
            for (Shape* s : loaded) {
                delete s;
            }
            return false;
        }
        Shape* s = codec->readPayload(is);
        if (!s) {
            for (Shape* x : loaded) {
                delete x;
            }
            return false;
        }
        loaded.push_back(s);
    }
    list.clear();
    for (Shape* s : loaded) {
        list.add(s);
    }
    return true;
}

bool tryLoadLegacyV1(ShapeList& list, std::istream& is) {
    std::vector<uint8_t> raw;
    const std::streampos pos = is.tellg();
    if (pos < 0) {
        return false;
    }
    is.seekg(0, std::ios::end);
    const std::streamoff end = is.tellg();
    if (end <= 0) {
        return false;
    }
    is.seekg(0, std::ios::beg);
    raw.resize(static_cast<size_t>(end));
    is.read(reinterpret_cast<char*>(raw.data()), end);
    if (is.fail()) {
        return false;
    }
    return deserializeRawList(list, raw);
}

} // namespace

void ShapeBinaryRegistry::registerCodec(std::unique_ptr<IShapeBinaryCodec> codec) {
    if (!codec) {
        return;
    }
    codecs()[codec->tag()] = std::move(codec);
}

const IShapeBinaryCodec* ShapeBinaryRegistry::find(uint32_t tag) {
    const auto it = codecs().find(tag);
    return it == codecs().end() ? nullptr : it->second.get();
}

std::string ShapeBinaryRegistry::listLabelFor(const Shape* shape) {
    if (!shape) {
        return "?";
    }
    const IShapeBinaryCodec* c = find(shape->binaryTypeTag());
    return c ? std::string(c->listLabel()) : std::string("unknown");
}

bool ShapeBinaryRegistry::saveListToFile(const ShapeList& list, const char* path) {
    std::ofstream os(path, std::ios::binary | std::ios::trunc);
    if (!os) {
        return false;
    }
    std::vector<uint8_t> payload;
    if (!serializeRawList(list, payload)) {
        return false;
    }

    std::vector<std::string> applied;
    std::string pipelineError;
    if (!StorageProcessorRegistry::applySavePipeline(payload, applied, pipelineError)) {
        return false;
    }

    writeU32(os, kFileMagic);
    writeU32(os, kFileVersion);
    writeU32(os, static_cast<uint32_t>(applied.size()));
    for (const std::string& id : applied) {
        writeString(os, id);
    }
    writeU32(os, static_cast<uint32_t>(payload.size()));
    writeBytes(os, payload);
    return static_cast<bool>(os);
}

bool ShapeBinaryRegistry::loadListFromFile(ShapeList& list, const char* path) {
    std::ifstream is(path, std::ios::binary);
    if (!is) {
        return false;
    }
    uint32_t magic = 0;
    uint32_t ver = 0;
    if (!readU32(is, magic)) {
        return false;
    }
    if (!readU32(is, ver)) {
        return false;
    }
    if (magic == kRawMagic && ver == kRawVersion) {
        is.seekg(0, std::ios::beg);
        return tryLoadLegacyV1(list, is);
    }
    if (magic != kFileMagic || ver != kFileVersion) {
        return false;
    }

    uint32_t chainCount = 0;
    if (!readU32(is, chainCount) || chainCount > 64) {
        return false;
    }
    std::vector<std::string> chain;
    chain.reserve(chainCount);
    for (uint32_t i = 0; i < chainCount; ++i) {
        std::string id;
        if (!readString(is, id)) {
            return false;
        }
        chain.push_back(id);
    }

    uint32_t payloadSize = 0;
    if (!readU32(is, payloadSize) || payloadSize > (256u * 1024u * 1024u)) {
        return false;
    }
    std::vector<uint8_t> payload;
    if (!readBytes(is, payload, payloadSize)) {
        return false;
    }

    std::string err;
    if (!StorageProcessorRegistry::applyLoadPipelineByIdList(chain, payload, err)) {
        return false;
    }
    return deserializeRawList(list, payload);
}
