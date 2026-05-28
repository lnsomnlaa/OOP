#include "ShapeBinaryRegistry.h"
#include "Shape.h"
#include "ShapeList.h"
#include <algorithm>
#include <fstream>
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
    writeU32(os, kFileMagic);
    writeU32(os, kFileVersion);
    const uint32_t n = static_cast<uint32_t>(list.count());
    writeU32(os, n);
    for (uint32_t i = 0; i < n; ++i) {
        const Shape* s = list.at(i);
        if (!s) {
            return false;
        }
        const uint32_t tag = s->binaryTypeTag();
        const IShapeBinaryCodec* codec = find(tag);
        if (!codec) {
            return false;
        }
        writeU32(os, tag);
        codec->writePayload(os, s);
    }
    return static_cast<bool>(os);
}

bool ShapeBinaryRegistry::loadListFromFile(ShapeList& list, const char* path) {
    std::ifstream is(path, std::ios::binary);
    if (!is) {
        return false;
    }
    uint32_t magic = 0;
    uint32_t ver = 0;
    uint32_t n = 0;
    if (!readU32(is, magic) || magic != kFileMagic) {
        return false;
    }
    if (!readU32(is, ver) || ver != kFileVersion) {
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
        const IShapeBinaryCodec* codec = find(tag);
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
