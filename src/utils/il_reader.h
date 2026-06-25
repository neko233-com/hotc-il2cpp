#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace hotc {

struct ILAssembly {
    std::string name;
    std::vector<uint8_t> metadata;
    std::vector<uint8_t> il_data;
};

class ILReader {
public:
    ILReader() = default;
    ~ILReader() = default;

    bool Load(const std::string& path);
    bool LoadFromMemory(const uint8_t* data, size_t size);

    const ILAssembly& GetAssembly() const { return assembly_; }

    bool IsValid() const { return valid_; }
    std::string GetError() const { return error_; }

private:
    bool ParsePE(const uint8_t* data, size_t size);
    bool ParseMetadata(const uint8_t* data, size_t size);

    ILAssembly assembly_;
    bool valid_ = false;
    std::string error_;
};

} // namespace hotc
