#include "il_reader.h"
#include <fstream>

namespace hotc {

bool ILReader::Load(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        error_ = "Failed to open file: " + path;
        return false;
    }

    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);
    file.read(reinterpret_cast<char*>(buffer.data()), size);

    return LoadFromMemory(buffer.data(), size);
}

bool ILReader::LoadFromMemory(const uint8_t* data, size_t size) {
    if (size < 2) {
        error_ = "Invalid IL data";
        return false;
    }

    // Simple IL verification
    if (data[0] != 0x42 || data[1] != 0x53) { // BS
        error_ = "Invalid IL signature";
        return false;
    }

    assembly_.il_data.assign(data, data + size);
    valid_ = true;
    return true;
}

bool ILReader::ParsePE(const uint8_t* data, size_t size) {
    // PE header parsing would go here
    return true;
}

bool ILReader::ParseMetadata(const uint8_t* data, size_t size) {
    // Metadata parsing would go here
    return true;
}

} // namespace hotc
