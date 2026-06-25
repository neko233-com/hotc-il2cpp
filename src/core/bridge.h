#pragma once

#include "platform.h"
#include "runtime.h"
#include <cstdint>
#include <vector>
#include <functional>
#include <cstring>

namespace hotc {

// Method signature for native bridge calls
using NativeMethod = void (*)(void* obj, void** args, void* result);
using NativeInvoker = void (*)(void* method, void* obj, void** args, void* result);

// Method bridge for native-to-managed calls
class MethodBridge {
public:
    MethodBridge() = default;
    ~MethodBridge() = default;
    
    // Register native method implementation
    void RegisterNative(uint32_t method_index, NativeMethod func);
    void RegisterInvoker(uint32_t method_index, NativeInvoker func);
    
    // Call managed method from native
    void CallManaged(uint32_t method_index, void* obj, void** args, void* result);
    
    // Call native method from managed
    void CallNative(uint32_t method_index, void* obj, void** args, void* result);
    
    // Check if method is native
    bool IsNative(uint32_t method_index) const;
    
    // Get native function pointer
    NativeMethod GetNativeFunc(uint32_t method_index) const;
    
private:
    std::unordered_map<uint32_t, NativeMethod> native_methods_;
    std::unordered_map<uint32_t, NativeInvoker> native_invokers_;
};

// Virtual dispatch table for efficient virtual calls
class VirtualDispatchTable {
public:
    VirtualDispatchTable() = default;
    ~VirtualDispatchTable() = default;
    
    // Initialize vtable for type
    void InitVTable(uint32_t type_index, uint32_t vtable_size);
    
    // Set vtable slot
    void SetSlot(uint32_t type_index, uint32_t slot, void* func);
    
    // Get vtable entry
    void* GetSlot(uint32_t type_index, uint32_t slot) const;
    
    // Inline cache for virtual dispatch
    struct VTableCache {
        uint32_t type_id;
        uint32_t version;
        void** vtable_ptr;
    };
    
    // Fast virtual dispatch with inline caching
    void* DispatchVirtual(uint32_t type_id, uint32_t slot, VTableCache& cache);
    
private:
    struct VTable {
        std::vector<void*> slots;
        uint32_t version = 0;
    };
    
    std::unordered_map<uint32_t, VTable> vtables_;
};

// Interface dispatch table
class InterfaceDispatchTable {
public:
    InterfaceDispatchTable() = default;
    ~InterfaceDispatchTable() = default;
    
    // Register interface implementation
    void RegisterInterface(uint32_t interface_type, uint32_t implementing_type, uint32_t slot, void* func);
    
    // Dispatch interface call
    void* DispatchInterface(uint32_t interface_type, uint32_t implementing_type, uint32_t slot);
    
private:
    struct InterfaceEntry {
        uint32_t implementing_type;
        uint32_t slot;
        void* func;
    };
    
    std::unordered_map<uint32_t, std::vector<InterfaceEntry>> interface_map_;
};

// Generic instantiation cache
class GenericCache {
public:
    GenericCache() = default;
    ~GenericCache() = default;
    
    // Get or create generic instantiation
    uint32_t GetInstantiation(uint32_t generic_method, const std::vector<uint32_t>& type_args);
    
    // Check if instantiation exists
    bool HasInstantiation(uint32_t generic_method, const std::vector<uint32_t>& type_args) const;
    
    // Clear cache
    void Clear();
    
private:
    struct GenericInstKey {
        uint32_t method;
        std::vector<uint32_t> type_args;
        
        bool operator==(const GenericInstKey& other) const {
            return method == other.method && type_args == other.type_args;
        }
    };
    
    struct GenericInstKeyHash {
        size_t operator()(const GenericInstKey& key) const {
            size_t h = std::hash<uint32_t>()(key.method);
            for (auto t : key.type_args) {
                h ^= std::hash<uint32_t>()(t) + 0x9e3779b9 + (h << 6) + (h >> 2);
            }
            return h;
        }
    };
    
    std::unordered_map<GenericInstKey, uint32_t, GenericInstKeyHash> cache_;
};

} // namespace hotc
