#pragma once
#include <windows.h>
#include <cstdint>
#include <functional>
#include <unordered_map>

class Shape;

/**
 * Draws shapes via a registry keyed by binaryTypeTag().
 * Tag-based dispatch works for types defined in separate DLL plugins (no RTTI across modules).
 */
class ShapeRenderer {
public:
    using DrawFn = std::function<void(HDC, const Shape*)>;

    /** Associates a drawer with the shape's binary serialization tag. */
    static void registerDrawerForTag(uint32_t binaryTag, DrawFn fn);

    static void draw(HDC hdc, const Shape* shape);

private:
    static std::unordered_map<uint32_t, DrawFn>& drawersByTag();
};
