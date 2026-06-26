#pragma once

#include "platform.h"
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <string>
#include <functional>

namespace hotc {

// Generic sharing - key for generic instantiation
struct GenericInstKey {
    uint32_t method_token;
    std::vector<uint32_t> type_args;
    
    bool operator==(const GenericInstKey& o) const {
        return method_token == o.method_token && type_args == o.type_args;
    }
};

struct GenericInstKeyHash {
    size_t operator()(const GenericInstKey& k) const {
        size_t h = std::hash<uint32_t>()(k.method_token);
        for (auto t : k.type_args) {
            h ^= std::hash<uint32_t>()(t) + 0x9e3779b9 + (h << 6) + (h >> 2);
        }
        return h;
    }
};

// Generic method instantiation
struct GenericMethodInst {
    uint32_t method_index;
    std::vector<uint32_t> type_args;
    uint32_t shared_method_index; // index into shared method table
};

// Generic class instantiation
struct GenericClassInst {
    uint32_t type_index;
    std::vector<uint32_t type_args;
    uint32_t shared_type_index;
};

// Shared method table entry - maps generic instantiation to concrete method
struct SharedMethodEntry {
    uint32_t method_index;
    uint32_t type_index;
    uint32_t rgctx_index; // runtime generic context
    void* invoker;
    void* method_pointer;
};

// Runtime generic context
struct RGCTX {
    uint32_t type_index;
    uint32_t method_index;
    void* method_pointer;
    void* invoker;
};

// Generic sharing manager
class GenericSharingManager {
public:
    GenericSharingManager() = default;
    ~GenericSharingManager() = default;
    
    // Instantiate a generic method
    uint32_t InstantiateGenericMethod(uint32_t method_token, const std::vector<uint32_t>& type_args);
    
    // Instantiate a generic class
    uint32_t InstantiateGenericClass(uint32_t type_token, const std::vector<uint32_t>& type_args);
    
    // Get shared method for instantiation
    const SharedMethodEntry* GetSharedMethod(uint32_t method_token, const std::vector<uint32_t>& type_args) const;
    
    // Get shared type for instantiation
    const GenericClassInst* GetSharedType(uint32_t type_token, const std::vector<uint32_t>& type_args) const;
    
    // Register AOT generic method (pre-compiled by Unity)
    void RegisterAOTGenericMethod(uint32_t method_token, const std::vector<uint32_t>& type_args, void* method_pointer);
    
    // Check if method is AOT generic
    bool IsAOTGeneric(uint32_t method_token) const;
    
    // Get all instantiations for a generic method
    std::vector<GenericMethodInst> GetInstantiations(uint32_t method_token) const;
    
    // Statistics
    uint32_t GetGenericMethodCount() const { return static_cast<uint32_t>(generic_methods_.size()); }
    uint32_t GetGenericClassCount() const { return static_cast<uint32_t>(generic_classes_.size()); }
    
private:
    // Method instantiations
    std::unordered_map<GenericInstKey, uint32_t, GenericInstKeyHash> method_inst_cache_;
    std::vector<GenericMethodInst> generic_methods_;
    
    // Class instantiations
    std::unordered_map<GenericInstKey, uint32_t, GenericInstKeyHash> class_inst_cache_;
    std::vector<GenericClassInst> generic_classes_;
    
    // Shared method table
    std::vector<SharedMethodEntry> shared_methods_;
    
    // AOT generic methods
    std::unordered_map<uint64_t, void*> aot_generic_methods_;
    
    // RGCTX table
    std::vector<RGCTX> rgctx_table_;
};

} // namespace hotc
