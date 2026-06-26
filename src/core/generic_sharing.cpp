#include "generic_sharing.h"

namespace hotc {

uint32_t GenericSharingManager::InstantiateGenericMethod(uint32_t method_token, const std::vector<uint32_t>& type_args) {
    GenericInstKey key{method_token, type_args};
    
    auto it = method_inst_cache_.find(key);
    if (it != method_inst_cache_.end()) {
        return it->second;
    }
    
    // Create new instantiation
    uint32_t index = static_cast<uint32_t>(generic_methods_.size());
    GenericMethodInst inst;
    inst.method_index = method_token;
    inst.type_args = type_args;
    inst.shared_method_index = static_cast<uint32_t>(shared_methods_.size());
    
    generic_methods_.push_back(inst);
    method_inst_cache_[key] = index;
    
    // Create shared method entry
    SharedMethodEntry entry;
    entry.method_index = method_token;
    entry.type_index = type_args.empty() ? 0 : type_args[0];
    entry.rgctx_index = static_cast<uint32_t>(rgctx_table_.size());
    entry.invoker = nullptr;
    entry.method_pointer = nullptr;
    shared_methods_.push_back(entry);
    
    // Create RGCTX
    RGCTX ctx;
    ctx.type_index = entry.type_index;
    ctx.method_index = method_token;
    ctx.method_pointer = nullptr;
    ctx.invoker = nullptr;
    rgctx_table_.push_back(ctx);
    
    return index;
}

uint32_t GenericSharingManager::InstantiateGenericClass(uint32_t type_token, const std::vector<uint32_t>& type_args) {
    GenericInstKey key{type_token, type_args};
    
    auto it = class_inst_cache_.find(key);
    if (it != class_inst_cache_.end()) {
        return it->second;
    }
    
    uint32_t index = static_cast<uint32_t>(generic_classes_.size());
    GenericClassInst inst;
    inst.type_index = type_token;
    inst.type_args = type_args;
    inst.shared_type_index = index;
    
    generic_classes_.push_back(inst);
    class_inst_cache_[key] = index;
    
    return index;
}

const SharedMethodEntry* GenericSharingManager::GetSharedMethod(uint32_t method_token, const std::vector<uint32_t>& type_args) const {
    GenericInstKey key{method_token, type_args};
    
    auto it = method_inst_cache_.find(key);
    if (it == method_inst_cache_.end()) {
        return nullptr;
    }
    
    uint32_t idx = it->second;
    if (idx >= generic_methods_.size()) return nullptr;
    
    uint32_t shared_idx = generic_methods_[idx].shared_method_index;
    if (shared_idx >= shared_methods_.size()) return nullptr;
    
    return &shared_methods_[shared_idx];
}

const GenericClassInst* GenericSharingManager::GetSharedType(uint32_t type_token, const std::vector<uint32_t>& type_args) const {
    GenericInstKey key{type_token, type_args};
    
    auto it = class_inst_cache_.find(key);
    if (it == class_inst_cache_.end()) {
        return nullptr;
    }
    
    uint32_t idx = it->second;
    if (idx >= generic_classes_.size()) return nullptr;
    
    return &generic_classes_[idx];
}

void GenericSharingManager::RegisterAOTGenericMethod(uint32_t method_token, const std::vector<uint32_t>& type_args, void* method_pointer) {
    uint64_t key = method_token;
    for (auto t : type_args) {
        key = (key << 32) | t;
    }
    aot_generic_methods_[key] = method_pointer;
}

bool GenericSharingManager::IsAOTGeneric(uint32_t method_token) const {
    for (auto& [key, ptr] : aot_generic_methods_) {
        if ((key >> 32) == method_token || (key & 0xFFFFFFFF) == method_token) {
            return true;
        }
    }
    return false;
}

std::vector<GenericMethodInst> GenericSharingManager::GetInstantiations(uint32_t method_token) const {
    std::vector<GenericMethodInst> result;
    for (const auto& inst : generic_methods_) {
        if (inst.method_index == method_token) {
            result.push_back(inst);
        }
    }
    return result;
}

} // namespace hotc
