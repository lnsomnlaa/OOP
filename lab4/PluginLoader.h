#pragma once

#include <string>
#include <vector>
#include <windows.h>

/**
 * Loads shape plugin DLLs at runtime.
 * Each DLL must export: void __cdecl ShapesPlugin_OnLoad(HostServices* host);
 */
class PluginLoader {
public:
    static void loadPluginsFromDefaultFolder(HWND ownerForErrors);

    static bool loadPluginAtPath(const wchar_t* fullPath, HWND ownerForErrors);

    static const std::vector<std::wstring>& loadedModulePaths();

private:
    static bool tryLoadOne(const std::wstring& fullPath, HWND ownerForErrors);
};
