#pragma once

#include "platform.h"
#include <cstdint>
#include <vector>
#include <functional>

namespace hotc {

// Method bridge for native interop
// Handles PInvoke, MonoPInvokeCallback, and managed-to-native calls

// PInvoke bridge entry
struct PInvokeBridge {
    uint32_t method_token;
    void* native_func;
    uint32_t calling_convention; // 0 = cdecl, 1 = stdcall, 2 = fastcall
    bool is_vararg;
};

// MonoPInvokeCallback entry
struct MonoPInvokeCallbackEntry {
    uint32_t method_token;
    void* callback_func;
    uint32_t param_count;
    uint32_t return_type; // 0 = void, 1 = int, 2 = float, 3 = double, 4 = object
};

// Managed-to-native call bridge
struct ManagedToNativeBridge {
    uint32_t method_token;
    void* invoke_func;
    void* reverse_pinvoke_wrapper;
};

// Native-to-managed call bridge
struct NativeToManagedBridge {
    uint32_t method_token;
    void* wrapper_func;
    uint32_t param_count;
};

// Method bridge manager
class MethodBridgeManager {
public:
    MethodBridgeManager() = default;
    ~MethodBridgeManager() = default;
    
    // Register PInvoke
    void RegisterPInvoke(uint32_t method_token, void* native_func, uint32_t conv = 0, bool vararg = false);
    
    // Register MonoPInvokeCallback
    void RegisterMonoPInvokeCallback(uint32_t method_token, void* callback, uint32_t param_count, uint32_t return_type);
    
    // Register managed-to-native bridge
    void RegisterManagedToNative(uint32_t method_token, void* invoke_func, void* reverse_wrapper);
    
    // Register native-to-managed bridge
    void RegisterNativeToManaged(uint32_t method_token, void* wrapper, uint32_t param_count);
    
    // Lookup
    const PInvokeBridge* GetPInvoke(uint32_t method_token) const;
    const MonoPInvokeCallbackEntry* GetMonoPInvokeCallback(uint32_t method_token) const;
    const ManagedToNativeBridge* GetManagedToNative(uint32_t method_token) const;
    const NativeToManagedBridge* GetNativeToManaged(uint32_t method_token) const;
    
    // Call helpers
    void* CallPInvoke(uint32_t method_token, void** args, uint32_t arg_count);
    void* CallMonoPInvokeCallback(uint32_t method_token, void** args, uint32_t arg_count);
    void* CallManagedToNative(uint32_t method_token, void** args, uint32_t arg_count);
    void* CallNativeToManaged(uint32_t method_token, void** args, uint32_t arg_count);
    
    // Statistics
    uint32_t GetPInvokeCount() const { return static_cast<uint32_t>(pinvoke_map_.size()); }
    uint32_t GetMonoPInvokeCallbackCount() const { return static_cast<uint32_t>(mono_callback_map_.size()); }
    
private:
    std::unordered_map<uint32_t, PInvokeBridge> pinvoke_map_;
    std::unordered_map<uint32_t, MonoPInvokeCallbackEntry> mono_callback_map_;
    std::unordered_map<uint32_t, ManagedToNativeBridge> m2n_map_;
    std::unordered_map<uint32_t, NativeToManagedBridge> n2m_map_;
};

} // namespace hotc
