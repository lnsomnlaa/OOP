#pragma once

#include <windows.h>

/**
 * The HMAC key is read from "plugin_host.key" in the executable directory (raw bytes, up to 64 bytes).
 * The signed message is: sha256 + "|" + expires (ASCII, same values as in the file).
 *
 * The .sig file is mandatory. If it is missing, the plugin is rejected.
 * If the .sig file exists, all three lines must validate or the plugin is rejected.
 */
bool PluginTrust_VerifyOrWarn(const wchar_t* dllPath, HWND owner);
