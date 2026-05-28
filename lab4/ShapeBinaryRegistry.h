#pragma once
#include <windows.h>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

class Shape;
class ShapeList;

/**
 * Per-type binary codec + property editor. New shapes register here; no switch on type in core I/O.
 */
class IShapeBinaryCodec
{
public:
    virtual ~IShapeBinaryCodec() = default;
    virtual uint32_t tag() const = 0;
    virtual const char *listLabel() const = 0;
    virtual void writePayload(std::ostream &os, const Shape *shape) const = 0;
    virtual Shape *readPayload(std::istream &is) const = 0;
    virtual bool editProperties(HWND owner, Shape *shape) const = 0;
};

class ShapeBinaryRegistry
{
public:
    static void registerCodec(std::unique_ptr<IShapeBinaryCodec> codec);
    static const IShapeBinaryCodec *find(uint32_t tag);
    static std::string listLabelFor(const Shape *shape);

    static constexpr uint32_t kFileMagic = 0x3342414Cu;
    static constexpr uint32_t kFileVersion = 1;

    static bool saveListToFile(const ShapeList &list, const char *path);
    static bool loadListFromFile(ShapeList &list, const char *path);
};
