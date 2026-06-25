#pragma once

#include "../core/type_system.h"
#include "../core/interpreter.h"
#include <cstdint>

namespace hotc {

class UnityBridge {
public:
    UnityBridge(TypeSystem& types, Interpreter& interpreter);
    ~UnityBridge() = default;

    void RegisterAllCoreModule();
    void RegisterCoreModuleTypes();
    void RegisterCoreModuleMethods();

private:
    void RegisterGameObject();
    void RegisterTransform();
    void RegisterComponent();
    void RegisterObject();
    void RegisterBehaviour();
    void RegisterMonoBehaviour();

    static void* GameObject_Create(void** args, uint32_t arg_count);
    static void* GameObject_Find(void** args, uint32_t arg_count);
    static void* GameObject_get_name(void** args, uint32_t arg_count);
    static void* GameObject_set_name(void** args, uint32_t arg_count);
    static void* GameObject_get_transform(void** args, uint32_t arg_count);
    static void* GameObject_AddComponent(void** args, uint32_t arg_count);
    static void* GameObject_GetComponent(void** args, uint32_t arg_count);
    static void* GameObject_Destroy(void** args, uint32_t arg_count);

    static void* Transform_get_position(void** args, uint32_t arg_count);
    static void* Transform_set_position(void** args, uint32_t arg_count);
    static void* Transform_get_rotation(void** args, uint32_t arg_count);
    static void* Transform_set_rotation(void** args, uint32_t arg_count);
    static void* Transform_get_scale(void** args, uint32_t arg_count);
    static void* Transform_set_scale(void** args, uint32_t arg_count);
    static void* Transform_get_parent(void** args, uint32_t arg_count);
    static void* Transform_set_parent(void** args, uint32_t arg_count);

    TypeSystem& type_system_;
    Interpreter& interpreter_;
};

} // namespace hotc
