#include "PluginLoader.h"

#include "HostServices.h"
#include "PluginTrust.h"

#include <shellapi.h>

#include <sstream>
#include <vector>

#pragma comment(lib, "shell32.lib")

namespace {

using ShapesPluginOnLoadFn = void(__cdecl*)(HostServices*);

std::vector<HMODULE> g_modules;
std::vector<std::wstring> g_paths;

/**
 * Returns directory of the current executable (no trailing slash).
 */
std::wstring exeDirectory() {
    wchar_t exe[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, exe, MAX_PATH);
    std::wstring dir(exe);
    const size_t slash = dir.find_last_of(L"\\/");
    if (slash != std::wstring::npos) {
        dir.resize(slash);
    }
    return dir;
}

std::wstring pluginsSearchPattern() {
    return exeDirectory() + L"\\plugins\\*.dll";
}

void appendError(HWND owner, const std::wstring& msg) {
    if (owner) {
        MessageBoxW(owner, msg.c_str(), L"Plugin loader", MB_OK | MB_ICONWARNING);
    }
}

}

const std::vector<std::wstring>& PluginLoader::loadedModulePaths() {
    return g_paths;
}

bool PluginLoader::tryLoadOne(const std::wstring& fullPath, HWND ownerForErrors) {
    if (!PluginTrust_VerifyOrWarn(fullPath.c_str(), ownerForErrors)) {
        return false;
    }

    HMODULE mod = LoadLibraryW(fullPath.c_str());
    if (!mod) {
        std::wstringstream ss;
        ss << L"LoadLibrary failed:\n" << fullPath;
        appendError(ownerForErrors, ss.str());
        return false;
    }

    auto* entry = reinterpret_cast<ShapesPluginOnLoadFn>(GetProcAddress(mod, "ShapesPlugin_OnLoad"));
    if (!entry) {
        FreeLibrary(mod);
        appendError(ownerForErrors, L"Export ShapesPlugin_OnLoad not found in:\n" + fullPath);
        return false;
    }

    entry(lab4_GetHostServices());
    g_modules.push_back(mod);
    g_paths.push_back(fullPath);
    return true;
}

bool PluginLoader::loadPluginAtPath(const wchar_t* fullPath, HWND ownerForErrors) {
    if (!fullPath || !fullPath[0]) {
        return false;
    }
    return tryLoadOne(fullPath, ownerForErrors);
}

void PluginLoader::loadPluginsFromDefaultFolder(HWND ownerForErrors) {
    const std::wstring pattern = pluginsSearchPattern();
    const std::wstring plugDir = exeDirectory() + L"\\plugins";
    WIN32_FIND_DATAW fd = {};
    HANDLE hFind = FindFirstFileW(pattern.c_str(), &fd);
    if (hFind == INVALID_HANDLE_VALUE) {
        return;
    }
    do {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            continue;
        }
        const std::wstring full = plugDir + L"\\" + fd.cFileName;
        tryLoadOne(full, ownerForErrors);
    } while (FindNextFileW(hFind, &fd));
    FindClose(hFind);
}
