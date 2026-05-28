#pragma once
#include <cstdint>

/** Unique binary type tags (explicit integers, endian-independent). */
namespace ShapeTags {
constexpr uint32_t Line = 1;
constexpr uint32_t Rect = 2;
constexpr uint32_t Ellipse = 3;
constexpr uint32_t Circle = 4;
constexpr uint32_t Triangle = 5;
constexpr uint32_t Point = 6;
constexpr uint32_t Polygon = 7;
constexpr uint32_t Cube3D = 8;
constexpr uint32_t Sphere3D = 9;
constexpr uint32_t Tetra3D = 10;

/**
 * Lowest tag value reserved for third-party plugins (must not overlap built-in tags 1..10).
 * Plugin authors should use PluginTagMin + N where N is documented for their module.
 */
constexpr uint32_t PluginTagMin = 0x1000u;
} // namespace ShapeTags
