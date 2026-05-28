#include "PluginTrust.h"

#include <bcrypt.h>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#pragma comment(lib, "bcrypt.lib")

namespace {

bool readFileBytes(const std::wstring& path, std::vector<unsigned char>& out) {
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        return false;
    }
    f.seekg(0, std::ios::end);
    const std::streamoff len = f.tellg();
    if (len <= 0) {
        return false;
    }
    f.seekg(0, std::ios::beg);
    out.resize(static_cast<size_t>(len));
    f.read(reinterpret_cast<char*>(out.data()), len);
    return static_cast<bool>(f);
}

bool sha256Bytes(const unsigned char* data, size_t len, unsigned char out32[32]) {
    BCRYPT_ALG_HANDLE alg = nullptr;
    BCRYPT_HASH_HANDLE hash = nullptr;
    DWORD cbHash = 0;
    DWORD cbData = 0;
    DWORD cbHashObject = 0;
    PUCHAR hashObject = nullptr;
    bool ok = false;

    if (!BCRYPT_SUCCESS(BCryptOpenAlgorithmProvider(&alg, BCRYPT_SHA256_ALGORITHM, nullptr, 0))) {
        return false;
    }
    if (!BCRYPT_SUCCESS(BCryptGetProperty(alg, BCRYPT_OBJECT_LENGTH, reinterpret_cast<PUCHAR>(&cbHashObject),
                                           sizeof(cbHashObject), &cbData, 0))) {
        BCryptCloseAlgorithmProvider(alg, 0);
        return false;
    }
    hashObject = static_cast<PUCHAR>(HeapAlloc(GetProcessHeap(), 0, cbHashObject));
    if (!hashObject) {
        BCryptCloseAlgorithmProvider(alg, 0);
        return false;
    }
    if (!BCRYPT_SUCCESS(BCryptGetProperty(alg, BCRYPT_HASH_LENGTH, reinterpret_cast<PUCHAR>(&cbHash), sizeof(cbHash),
                                           &cbData, 0)) ||
        cbHash != 32) {
        HeapFree(GetProcessHeap(), 0, hashObject);
        BCryptCloseAlgorithmProvider(alg, 0);
        return false;
    }
    if (!BCRYPT_SUCCESS(BCryptCreateHash(alg, &hash, hashObject, cbHashObject, nullptr, 0, 0))) {
        HeapFree(GetProcessHeap(), 0, hashObject);
        BCryptCloseAlgorithmProvider(alg, 0);
        return false;
    }
    if (!BCRYPT_SUCCESS(BCryptHashData(hash, const_cast<PUCHAR>(data), static_cast<ULONG>(len), 0))) {
        BCryptDestroyHash(hash);
        HeapFree(GetProcessHeap(), 0, hashObject);
        BCryptCloseAlgorithmProvider(alg, 0);
        return false;
    }
    if (!BCRYPT_SUCCESS(BCryptFinishHash(hash, out32, 32, 0))) {
        BCryptDestroyHash(hash);
        HeapFree(GetProcessHeap(), 0, hashObject);
        BCryptCloseAlgorithmProvider(alg, 0);
        return false;
    }
    ok = true;
    BCryptDestroyHash(hash);
    HeapFree(GetProcessHeap(), 0, hashObject);
    BCryptCloseAlgorithmProvider(alg, 0);
    return ok;
}

/**
 * Converts 32 digest bytes to lowercase hex string.
 */
std::string toHexLower(const unsigned char* buf, size_t n) {
    static const char* kHex = "0123456789abcdef";
    std::string s;
    s.resize(n * 2);
    for (size_t i = 0; i < n; ++i) {
        s[i * 2] = kHex[(buf[i] >> 4) & 0xF];
        s[i * 2 + 1] = kHex[buf[i] & 0xF];
    }
    return s;
}

bool equalsNoCase(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) {
        return false;
    }
    for (size_t i = 0; i < a.size(); ++i) {
        if (std::tolower(static_cast<unsigned char>(a[i])) != std::tolower(static_cast<unsigned char>(b[i]))) {
            return false;
        }
    }
    return true;
}

/**
 * Computes HMAC-SHA256 using CNG (secret key passed into BCryptCreateHash).
 */
bool hmacSha256(const std::vector<unsigned char>& key, const std::string& message, unsigned char out32[32]) {
    BCRYPT_ALG_HANDLE alg = nullptr;
    BCRYPT_HASH_HANDLE hash = nullptr;
    DWORD cbHashObject = 0;
    DWORD cbData = 0;
    DWORD cbHash = 0;
    PUCHAR hashObject = nullptr;

    if (!BCRYPT_SUCCESS(BCryptOpenAlgorithmProvider(&alg, BCRYPT_SHA256_ALGORITHM, nullptr, BCRYPT_ALG_HANDLE_HMAC_FLAG))) {
        return false;
    }
    if (!BCRYPT_SUCCESS(BCryptGetProperty(alg, BCRYPT_OBJECT_LENGTH, reinterpret_cast<PUCHAR>(&cbHashObject),
                                           sizeof(cbHashObject), &cbData, 0))) {
        BCryptCloseAlgorithmProvider(alg, 0);
        return false;
    }
    hashObject = static_cast<PUCHAR>(HeapAlloc(GetProcessHeap(), 0, cbHashObject));
    if (!hashObject) {
        BCryptCloseAlgorithmProvider(alg, 0);
        return false;
    }
    std::vector<UCHAR> keyCopy(key.begin(), key.end());
    if (!BCRYPT_SUCCESS(BCryptGetProperty(alg, BCRYPT_HASH_LENGTH, reinterpret_cast<PUCHAR>(&cbHash), sizeof(cbHash),
                                           &cbData, 0)) ||
        cbHash != 32) {
        HeapFree(GetProcessHeap(), 0, hashObject);
        BCryptCloseAlgorithmProvider(alg, 0);
        return false;
    }
    if (!BCRYPT_SUCCESS(BCryptCreateHash(alg, &hash, hashObject, cbHashObject, keyCopy.data(),
                                         static_cast<ULONG>(keyCopy.size()), 0))) {
        HeapFree(GetProcessHeap(), 0, hashObject);
        BCryptCloseAlgorithmProvider(alg, 0);
        return false;
    }
    const auto* msgBytes = reinterpret_cast<const unsigned char*>(message.data());
    if (!BCRYPT_SUCCESS(
            BCryptHashData(hash, const_cast<PUCHAR>(msgBytes), static_cast<ULONG>(message.size()), 0))) {
        BCryptDestroyHash(hash);
        HeapFree(GetProcessHeap(), 0, hashObject);
        BCryptCloseAlgorithmProvider(alg, 0);
        return false;
    }
    if (!BCRYPT_SUCCESS(BCryptFinishHash(hash, out32, 32, 0))) {
        BCryptDestroyHash(hash);
        HeapFree(GetProcessHeap(), 0, hashObject);
        BCryptCloseAlgorithmProvider(alg, 0);
        return false;
    }
    BCryptDestroyHash(hash);
    HeapFree(GetProcessHeap(), 0, hashObject);
    BCryptCloseAlgorithmProvider(alg, 0);
    return true;
}

std::wstring exeDirectory() {
    wchar_t buf[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, buf, MAX_PATH);
    std::wstring p(buf);
    const size_t slash = p.find_last_of(L"\\/");
    if (slash != std::wstring::npos) {
        p.resize(slash);
    }
    return p;
}

bool loadHostKey(std::vector<unsigned char>& keyOut) {
    const std::wstring keyPath = exeDirectory() + L"\\plugin_host.key";
    return readFileBytes(keyPath, keyOut) && !keyOut.empty();
}

}

bool PluginTrust_VerifyOrWarn(const wchar_t* dllPath, HWND owner) {
    if (!dllPath) {
        return false;
    }
    std::wstring sigPath(dllPath);
    sigPath += L".sig";

    std::vector<unsigned char> sigFile;
    if (!readFileBytes(sigPath, sigFile)) {
        MessageBoxW(owner, L"Missing .sig file next to plugin DLL. Unsigned plugins are not allowed.",
                    L"Plugin trust", MB_ICONERROR);
        return false;
    }

    std::string text(reinterpret_cast<const char*>(sigFile.data()), sigFile.size());
    std::string expiresLine;
    std::string shaLine;
    std::string hmacLine;
    std::istringstream iss(text);
    std::string line;
    while (std::getline(iss, line)) {
        if (line.size() >= 2 && line.back() == '\r') {
            line.pop_back();
        }
        const size_t eq = line.find('=');
        if (eq == std::string::npos) {
            continue;
        }
        const std::string k = line.substr(0, eq);
        const std::string v = line.substr(eq + 1);
        if (k == "expires") {
            expiresLine = v;
        } else if (k == "sha256") {
            shaLine = v;
        } else if (k == "hmac") {
            hmacLine = v;
        }
    }
    if (expiresLine.empty() || shaLine.empty() || hmacLine.empty()) {
        MessageBoxW(owner, L"Invalid .sig file (need expires=, sha256=, hmac=).", L"Plugin trust", MB_ICONERROR);
        return false;
    }

    std::vector<unsigned char> dllBytes;
    if (!readFileBytes(dllPath, dllBytes)) {
        MessageBoxW(owner, L"Cannot read plugin DLL for integrity check.", L"Plugin trust", MB_ICONERROR);
        return false;
    }
    unsigned char digest[32] = {};
    if (!sha256Bytes(dllBytes.data(), dllBytes.size(), digest)) {
        MessageBoxW(owner, L"SHA-256 computation failed.", L"Plugin trust", MB_ICONERROR);
        return false;
    }
    const std::string computed = toHexLower(digest, 32);
    if (!equalsNoCase(computed, shaLine)) {
        MessageBoxW(owner, L"Plugin file hash does not match .sig (tampered?).", L"Plugin trust", MB_ICONERROR);
        return false;
    }

    char* endPtr = nullptr;
    const long long exp = std::strtoll(expiresLine.c_str(), &endPtr, 10);
    if (!endPtr || *endPtr != '\0' || exp <= 0) {
        MessageBoxW(owner, L"Invalid expires= in .sig file.", L"Plugin trust", MB_ICONERROR);
        return false;
    }
    const std::time_t now = std::time(nullptr);
    if (static_cast<long long>(now) > exp) {
        MessageBoxW(owner, L"Plugin signature expired (activation time window).", L"Plugin trust", MB_ICONERROR);
        return false;
    }

    std::vector<unsigned char> key;
    if (!loadHostKey(key)) {
        MessageBoxW(owner, L"plugin_host.key missing next to exe; cannot verify HMAC.", L"Plugin trust", MB_ICONERROR);
        return false;
    }
    const std::string message = shaLine + "|" + expiresLine;
    unsigned char mac[32] = {};
    if (!hmacSha256(key, message, mac)) {
        MessageBoxW(owner, L"HMAC computation failed.", L"Plugin trust", MB_ICONERROR);
        return false;
    }
    const std::string macHex = toHexLower(mac, 32);
    if (!equalsNoCase(macHex, hmacLine)) {
        MessageBoxW(owner, L"HMAC mismatch (wrong key or forged .sig).", L"Plugin trust", MB_ICONERROR);
        return false;
    }
    return true;
}
