#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <windows.h>

/**
 * Pluggable byte-level processor used in file save/load pipeline.
 * The host applies enabled processors before writing to disk and
 * applies reverse decode after reading.
 */
class IStorageProcessor {
public:
    virtual ~IStorageProcessor() = default;
    virtual const char* id() const = 0;
    virtual const char* displayName() const = 0;
    virtual bool encode(const std::vector<uint8_t>& in, std::vector<uint8_t>& out, std::string& errorOut) const = 0;
    virtual bool decode(const std::vector<uint8_t>& in, std::vector<uint8_t>& out, std::string& errorOut) const = 0;
    virtual bool configure(HWND owner) = 0;
};

class StorageProcessorRegistry {
public:
    static void registerProcessor(std::unique_ptr<IStorageProcessor> processor);

    static std::vector<std::string> enabledProcessorIds();

    static bool applySavePipeline(std::vector<uint8_t>& inOutData, std::vector<std::string>& appliedIds, std::string& errorOut);

    static bool applyLoadPipelineByIdList(const std::vector<std::string>& ids, std::vector<uint8_t>& inOutData, std::string& errorOut);

    static bool hasProcessors();

    static void showSettingsDialog(HWND owner);
};
