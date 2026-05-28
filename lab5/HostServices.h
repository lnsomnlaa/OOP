#pragma once

#include <functional>
#include <memory>
#include <string>
#include <windows.h>

class Shape;
class IShapeCreator;
class IMouseTool;
class IShapeBinaryCodec;
class IStorageProcessor;

/**
 * Host-facing registration API passed into plugins.
 * Implemented inside the main executable; plugins call through a single vtable pointer.
 * This avoids linking the plugin against host object files while still using C++ interfaces.
 */
class HostServices {
public:
    virtual ~HostServices() = default;

    virtual void registerCreator(std::unique_ptr<IShapeCreator> creator) = 0;

    virtual void registerTool(std::unique_ptr<IMouseTool> tool) = 0;

    virtual void registerCodec(std::unique_ptr<IShapeBinaryCodec> codec) = 0;

    virtual void registerStorageProcessor(std::unique_ptr<IStorageProcessor> processor) = 0;

    virtual void registerDrawerForTag(uint32_t binaryTag, std::function<void(HDC, const Shape*)> fn) = 0;

    virtual bool editParamsDialog(HWND owner, const char* windowTitle, std::string& textInOut) = 0;
};

HostServices* lab4_GetHostServices();
