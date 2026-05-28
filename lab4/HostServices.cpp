#include "HostServices.h"
#include "Lab3Dialogs.h"
#include "ShapeBinaryRegistry.h"
#include "ShapeFactory.h"
#include "ShapeRenderer.h"

namespace {

/**
 * Forwards registration calls to the existing static registries in the host process.
 */
class HostServicesImpl final : public HostServices {
public:
    void registerCreator(std::unique_ptr<IShapeCreator> creator) override {
        ShapeFactory::registerCreator(std::move(creator));
    }

    void registerTool(std::unique_ptr<IMouseTool> tool) override {
        MouseToolRegistry::registerTool(std::move(tool));
    }

    void registerCodec(std::unique_ptr<IShapeBinaryCodec> codec) override {
        ShapeBinaryRegistry::registerCodec(std::move(codec));
    }

    void registerDrawerForTag(uint32_t binaryTag, std::function<void(HDC, const Shape*)> fn) override {
        ShapeRenderer::registerDrawerForTag(binaryTag, std::move(fn));
    }

    bool editParamsDialog(HWND owner, const char* windowTitle, std::string& textInOut) override {
        return Lab3_EditTextDialog(owner, windowTitle, textInOut);
    }
};

HostServicesImpl g_hostServices;

} // namespace

HostServices* lab4_GetHostServices() {
    return &g_hostServices;
}
