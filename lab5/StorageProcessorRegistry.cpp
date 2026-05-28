#include "StorageProcessorRegistry.h"

#include "Lab3Dialogs.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <unordered_map>

namespace {

struct ProcessorEntry {
    std::unique_ptr<IStorageProcessor> processor;
    bool enabled = true;
};

std::vector<ProcessorEntry>& entries() {
    static std::vector<ProcessorEntry> v;
    return v;
}

std::string lower(const std::string& s) {
    std::string out = s;
    for (char& c : out) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return out;
}

ProcessorEntry* findEntryById(const std::string& id) {
    const std::string key = lower(id);
    for (ProcessorEntry& e : entries()) {
        if (e.processor && lower(e.processor->id()) == key) {
            return &e;
        }
    }
    return nullptr;
}

const ProcessorEntry* findEntryByIdConst(const std::string& id) {
    const std::string key = lower(id);
    for (const ProcessorEntry& e : entries()) {
        if (e.processor && lower(e.processor->id()) == key) {
            return &e;
        }
    }
    return nullptr;
}

std::string buildSettingsText() {
    std::ostringstream oss;
    oss << "# Storage processors (archiving plugins)\r\n";
    oss << "# Commands: on <id>, off <id>, cfg <id>\r\n";
    oss << "# Keep header lines. Edit command lines only.\r\n";
    oss << "\r\n";
    for (const ProcessorEntry& e : entries()) {
        if (!e.processor) {
            continue;
        }
        oss << (e.enabled ? "[on ] " : "[off] ") << e.processor->id() << " - " << e.processor->displayName() << "\r\n";
    }
    return oss.str();
}

void trim(std::string& s) {
    const auto isSp = [](unsigned char c) { return std::isspace(c) != 0; };
    size_t b = 0;
    while (b < s.size() && isSp(static_cast<unsigned char>(s[b]))) {
        ++b;
    }
    size_t e = s.size();
    while (e > b && isSp(static_cast<unsigned char>(s[e - 1]))) {
        --e;
    }
    s = s.substr(b, e - b);
}

}

void StorageProcessorRegistry::registerProcessor(std::unique_ptr<IStorageProcessor> processor) {
    if (!processor) {
        return;
    }
    const std::string id = lower(processor->id());
    for (ProcessorEntry& e : entries()) {
        if (e.processor && lower(e.processor->id()) == id) {
            e.processor = std::move(processor);
            e.enabled = true;
            return;
        }
    }
    entries().push_back(ProcessorEntry{std::move(processor), true});
}

std::vector<std::string> StorageProcessorRegistry::enabledProcessorIds() {
    std::vector<std::string> ids;
    for (const ProcessorEntry& e : entries()) {
        if (e.processor && e.enabled) {
            ids.push_back(e.processor->id());
        }
    }
    return ids;
}

bool StorageProcessorRegistry::applySavePipeline(std::vector<uint8_t>& inOutData, std::vector<std::string>& appliedIds,
                                                 std::string& errorOut) {
    appliedIds.clear();
    for (const ProcessorEntry& e : entries()) {
        if (!e.processor || !e.enabled) {
            continue;
        }
        std::vector<uint8_t> out;
        if (!e.processor->encode(inOutData, out, errorOut)) {
            if (errorOut.empty()) {
                errorOut = "encode failed for processor: " + std::string(e.processor->id());
            }
            return false;
        }
        inOutData.swap(out);
        appliedIds.push_back(e.processor->id());
    }
    return true;
}

bool StorageProcessorRegistry::applyLoadPipelineByIdList(const std::vector<std::string>& ids, std::vector<uint8_t>& inOutData,
                                                         std::string& errorOut) {
    for (auto it = ids.rbegin(); it != ids.rend(); ++it) {
        const ProcessorEntry* e = findEntryByIdConst(*it);
        if (!e || !e->processor) {
            errorOut = "required processor not loaded: " + *it;
            return false;
        }
        std::vector<uint8_t> out;
        if (!e->processor->decode(inOutData, out, errorOut)) {
            if (errorOut.empty()) {
                errorOut = "decode failed for processor: " + std::string(e->processor->id());
            }
            return false;
        }
        inOutData.swap(out);
    }
    return true;
}

bool StorageProcessorRegistry::hasProcessors() {
    for (const ProcessorEntry& e : entries()) {
        if (e.processor) {
            return true;
        }
    }
    return false;
}

void StorageProcessorRegistry::showSettingsDialog(HWND owner) {
    std::string text = buildSettingsText();
    if (!Lab3_EditTextDialog(owner, "Plugin settings: storage processors", text)) {
        return;
    }
    std::istringstream iss(text);
    std::string line;
    while (std::getline(iss, line)) {
        trim(line);
        if (line.empty() || line[0] == '#') {
            continue;
        }
        if (line.size() >= 1 && line[0] == '[') {
            continue;
        }
        std::istringstream ls(line);
        std::string cmd;
        std::string id;
        ls >> cmd >> id;
        cmd = lower(cmd);
        if (id.empty()) {
            continue;
        }
        ProcessorEntry* e = findEntryById(id);
        if (!e || !e->processor) {
            continue;
        }
        if (cmd == "on") {
            e->enabled = true;
        } else if (cmd == "off") {
            e->enabled = false;
        } else if (cmd == "cfg") {
            e->processor->configure(owner);
        }
    }
}
