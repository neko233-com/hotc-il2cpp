#include "method_bridge.h"
#include <cstring>

namespace hotc {

void MethodBridgeManager::RegisterPInvoke(uint32_t method_token, void* native_func, uint32_t conv, bool vararg) {
    PInvokeBridge bridge;
    bridge.method_token = method_token;
    bridge.native_func = native_func;
    bridge.calling_convention = conv;
    bridge.is_vararg = vararg;
    pinvoke_map_[method_token] = bridge;
}

void MethodBridgeManager::RegisterMonoPInvokeCallback(uint32_t method_token, void* callback, uint32_t param_count, uint32_t return_type) {
    MonoPInvokeCallbackEntry entry;
    entry.method_token = method_token;
    entry.callback_func = callback;
    entry.param_count = param_count;
    entry.return_type = return_type;
    mono_callback_map_[method_token] = entry;
}

void MethodBridgeManager::RegisterManagedToNative(uint32_t method_token, void* invoke_func, void* reverse_wrapper) {
    ManagedToNativeBridge bridge;
    bridge.method_token = method_token;
    bridge.invoke_func = invoke_func;
    bridge.reverse_pinvoke_wrapper = reverse_wrapper;
    m2n_map_[method_token] = bridge;
}

void MethodBridgeManager::RegisterNativeToManaged(uint32_t method_token, void* wrapper, uint32_t param_count) {
    NativeToManagedBridge bridge;
    bridge.method_token = method_token;
    bridge.wrapper_func = wrapper;
    bridge.param_count = param_count;
    n2m_map_[method_token] = bridge;
}

const PInvokeBridge* MethodBridgeManager::GetPInvoke(uint32_t method_token) const {
    auto it = pinvoke_map_.find(method_token);
    return (it != pinvoke_map_.end()) ? &it->second : nullptr;
}

const MonoPInvokeCallbackEntry* MethodBridgeManager::GetMonoPInvokeCallback(uint32_t method_token) const {
    auto it = mono_callback_map_.find(method_token);
    return (it != mono_callback_map_.end()) ? &it->second : nullptr;
}

const ManagedToNativeBridge* MethodBridgeManager::GetManagedToNative(uint32_t method_token) const {
    auto it = m2n_map_.find(method_token);
    return (it != m2n_map_.end()) ? &it->second : nullptr;
}

const NativeToManagedBridge* MethodBridgeManager::GetNativeToManaged(uint32_t method_token) const {
    auto it = n2m_map_.find(method_token);
    return (it != n2m_map_.end()) ? &it->second : nullptr;
}

void* MethodBridgeManager::CallPInvoke(uint32_t method_token, void** args, uint32_t arg_count) {
    const auto* bridge = GetPInvoke(method_token);
    if (!bridge || !bridge->native_func) return nullptr;
    
    // Cast native function and call it
    // The actual calling convention handling depends on platform
    using FuncPtr = void* (*)(void**, uint32_t);
    FuncPtr func = reinterpret_cast<FuncPtr>(bridge->native_func);
    return func(args, arg_count);
}

void* MethodBridgeManager::CallMonoPInvokeCallback(uint32_t method_token, void** args, uint32_t arg_count) {
    const auto* entry = GetMonoPInvokeCallback(method_token);
    if (!entry || !entry->callback_func) return nullptr;
    
    using Callback = void* (*)(void**, uint32_t);
    Callback cb = reinterpret_cast<Callback>(entry->callback_func);
    return cb(args, arg_count);
}

void* MethodBridgeManager::CallManagedToNative(uint32_t method_token, void** args, uint32_t arg_count) {
    const auto* bridge = GetManagedToNative(method_token);
    if (!bridge || !bridge->invoke_func) return nullptr;
    
    using InvokeFunc = void* (*)(void**, uint32_t);
    InvokeFunc func = reinterpret_cast<InvokeFunc>(bridge->invoke_func);
    return func(args, arg_count);
}

void* MethodBridgeManager::CallNativeToManaged(uint32_t method_token, void** args, uint32_t arg_count) {
    const auto* bridge = GetNativeToManaged(method_token);
    if (!bridge || !bridge->wrapper_func) return nullptr;
    
    using WrapperFunc = void* (*)(void**, uint32_t);
    WrapperFunc func = reinterpret_cast<WrapperFunc>(bridge->wrapper_func);
    return func(args, arg_count);
}

} // namespace hotc
