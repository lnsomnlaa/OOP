#include "ShapeFactory.h"
#include "Shape.h"
#include <algorithm>
#include <cctype>
#include <unordered_map>

namespace {

std::string lower(std::string s) {
    for (char& c : s) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return s;
}

std::unordered_map<std::string, std::unique_ptr<IShapeCreator>>& creators() {
    static std::unordered_map<std::string, std::unique_ptr<IShapeCreator>> m;
    return m;
}

std::unordered_map<std::string, std::unique_ptr<IMouseTool>>& mouseTools() {
    static std::unordered_map<std::string, std::unique_ptr<IMouseTool>> m;
    return m;
}

std::unordered_map<std::string, size_t>& pointCountOverrides() {
    static std::unordered_map<std::string, size_t> m;
    return m;
}

} // namespace

void ShapeFactory::registerCreator(std::unique_ptr<IShapeCreator> c) {
    if (c) {
        creators()[lower(c->commandId())] = std::move(c);
    }
}

Shape* ShapeFactory::create(const std::string& id, const std::vector<int>& params) {
    const auto it = creators().find(lower(id));
    if (it == creators().end() || !it->second || params.size() < it->second->minParamCount()) {
        return nullptr;
    }
    return it->second->create(params);
}

std::vector<std::string> ShapeFactory::shapeIds() {
    std::vector<std::string> out;
    for (const auto& kv : creators()) {
        out.push_back(kv.first);
    }
    std::sort(out.begin(), out.end());
    return out;
}

void MouseToolRegistry::registerTool(std::unique_ptr<IMouseTool> t) {
    if (t) {
        mouseTools()[lower(t->commandId())] = std::move(t);
    }
}

const IMouseTool* MouseToolRegistry::find(const std::string& id) {
    const auto it = mouseTools().find(lower(id));
    return it == mouseTools().end() ? nullptr : it->second.get();
}

std::vector<std::string> MouseToolRegistry::toolIds() {
    std::vector<std::string> out;
    for (const auto& kv : mouseTools()) {
        out.push_back(kv.first);
    }
    std::sort(out.begin(), out.end());
    return out;
}

void MouseToolRegistry::setPointCountOverride(const std::string& id, size_t count) {
    const std::string key = lower(id);
    if (count == 0) {
        pointCountOverrides().erase(key);
        return;
    }
    pointCountOverrides()[key] = count;
}

size_t MouseToolRegistry::pointCountOr(const std::string& id, size_t fallback) {
    const std::string key = lower(id);
    const auto it = pointCountOverrides().find(key);
    if (it == pointCountOverrides().end()) {
        return fallback;
    }
    return it->second;
}
