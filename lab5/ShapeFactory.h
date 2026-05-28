#pragma once
#include <windows.h>
#include <memory>
#include <string>
#include <vector>

class Shape;

/**
 * Creates a shape from a command name and integer parameters
 */
class IShapeCreator
{
public:
    virtual ~IShapeCreator() = default;
    virtual Shape *create(const std::vector<int> &params) const = 0;
    virtual size_t minParamCount() const = 0;
    virtual const char *commandId() const = 0;
};

/**
 * Converts a sequence of mouse clicks into factory parameters for one tool
 */
class IMouseTool
{
public:
    virtual ~IMouseTool() = default;
    virtual size_t pointCount() const = 0;
    virtual const char *commandId() const = 0;
    virtual std::vector<int> toParams(const std::vector<POINT> &pts) const = 0;
};

class ShapeFactory
{
public:
    static void registerCreator(std::unique_ptr<IShapeCreator> creator);
    static Shape *create(const std::string &id, const std::vector<int> &params);
    static std::vector<std::string> shapeIds();
};

class MouseToolRegistry
{
public:
    static void registerTool(std::unique_ptr<IMouseTool> tool);
    static const IMouseTool *find(const std::string &id);
    static std::vector<std::string> toolIds();

    static void setPointCountOverride(const std::string &id, size_t count);

    static size_t pointCountOr(const std::string &id, size_t fallback);
};
