#include "unity_bridge.h"

namespace hotc {

UnityBridge::UnityBridge(TypeSystem& types, Interpreter& interpreter)
    : type_system_(types), interpreter_(interpreter) {
}

void UnityBridge::RegisterAllCoreModule() {
    RegisterCoreModuleTypes();
    RegisterCoreModuleMethods();
}

void UnityBridge::RegisterCoreModuleTypes() {
    RegisterObject();
    RegisterComponent();
    RegisterBehaviour();
    RegisterMonoBehaviour();
    RegisterGameObject();
    RegisterTransform();
}

void UnityBridge::RegisterCoreModuleMethods() {
    // GameObject methods
    interpreter_.RegisterNativeFunction("UnityEngine.GameObject::Create", GameObject_Create);
    interpreter_.RegisterNativeFunction("UnityEngine.GameObject::Find", GameObject_Find);
    interpreter_.RegisterNativeFunction("UnityEngine.GameObject::get_name", GameObject_get_name);
    interpreter_.RegisterNativeFunction("UnityEngine.GameObject::set_name", GameObject_set_name);
    interpreter_.RegisterNativeFunction("UnityEngine.GameObject::get_transform", GameObject_get_transform);
    interpreter_.RegisterNativeFunction("UnityEngine.GameObject::AddComponent", GameObject_AddComponent);
    interpreter_.RegisterNativeFunction("UnityEngine.GameObject::GetComponent", GameObject_GetComponent);
    interpreter_.RegisterNativeFunction("UnityEngine.GameObject::Destroy", GameObject_Destroy);

    // Transform methods
    interpreter_.RegisterNativeFunction("UnityEngine.Transform::get_position", Transform_get_position);
    interpreter_.RegisterNativeFunction("UnityEngine.Transform::set_position", Transform_set_position);
    interpreter_.RegisterNativeFunction("UnityEngine.Transform::get_rotation", Transform_get_rotation);
    interpreter_.RegisterNativeFunction("UnityEngine.Transform::set_rotation", Transform_set_rotation);
    interpreter_.RegisterNativeFunction("UnityEngine.Transform::get_scale", Transform_get_scale);
    interpreter_.RegisterNativeFunction("UnityEngine.Transform::set_scale", Transform_set_scale);
    interpreter_.RegisterNativeFunction("UnityEngine.Transform::get_parent", Transform_get_parent);
    interpreter_.RegisterNativeFunction("UnityEngine.Transform::set_parent", Transform_set_parent);
}

void UnityBridge::RegisterGameObject() {
    TypeInfo* game_object = type_system_.RegisterClassType("UnityEngine.GameObject", type_system_.GetObjectType());
    type_system_.RegisterField(game_object, "m_Name", type_system_.GetStringType(), 0);
    type_system_.RegisterField(game_object, "m_Transform", type_system_.GetObjectType(), 8);
}

void UnityBridge::RegisterTransform() {
    TypeInfo* transform = type_system_.RegisterClassType("UnityEngine.Transform", type_system_.GetObjectType());
    type_system_.RegisterField(transform, "m_Position", type_system_.GetObjectType(), 0);
    type_system_.RegisterField(transform, "m_Rotation", type_system_.GetObjectType(), 12);
    type_system_.RegisterField(transform, "m_Scale", type_system_.GetObjectType(), 24);
    type_system_.RegisterField(transform, "m_Parent", type_system_.GetObjectType(), 36);
}

void UnityBridge::RegisterComponent() {
    type_system_.RegisterClassType("UnityEngine.Component", type_system_.GetObjectType());
}

void UnityBridge::RegisterObject() {
    // Already registered in TypeSystem constructor
}

void UnityBridge::RegisterBehaviour() {
    type_system_.RegisterClassType("UnityEngine.Behaviour", type_system_.GetObjectType());
}

void UnityBridge::RegisterMonoBehaviour() {
    type_system_.RegisterClassType("UnityEngine.MonoBehaviour", type_system_.GetObjectType());
}

void* UnityBridge::GameObject_Create(void** args, uint32_t arg_count) {
    return nullptr; // Stub
}

void* UnityBridge::GameObject_Find(void** args, uint32_t arg_count) {
    return nullptr; // Stub
}

void* UnityBridge::GameObject_get_name(void** args, uint32_t arg_count) {
    return nullptr; // Stub
}

void* UnityBridge::GameObject_set_name(void** args, uint32_t arg_count) {
    return nullptr; // Stub
}

void* UnityBridge::GameObject_get_transform(void** args, uint32_t arg_count) {
    return nullptr; // Stub
}

void* UnityBridge::GameObject_AddComponent(void** args, uint32_t arg_count) {
    return nullptr; // Stub
}

void* UnityBridge::GameObject_GetComponent(void** args, uint32_t arg_count) {
    return nullptr; // Stub
}

void* UnityBridge::GameObject_Destroy(void** args, uint32_t arg_count) {
    return nullptr; // Stub
}

void* UnityBridge::Transform_get_position(void** args, uint32_t arg_count) {
    return nullptr; // Stub
}

void* UnityBridge::Transform_set_position(void** args, uint32_t arg_count) {
    return nullptr; // Stub
}

void* UnityBridge::Transform_get_rotation(void** args, uint32_t arg_count) {
    return nullptr; // Stub
}

void* UnityBridge::Transform_set_rotation(void** args, uint32_t arg_count) {
    return nullptr; // Stub
}

void* UnityBridge::Transform_get_scale(void** args, uint32_t arg_count) {
    return nullptr; // Stub
}

void* UnityBridge::Transform_set_scale(void** args, uint32_t arg_count) {
    return nullptr; // Stub
}

void* UnityBridge::Transform_get_parent(void** args, uint32_t arg_count) {
    return nullptr; // Stub
}

void* UnityBridge::Transform_set_parent(void** args, uint32_t arg_count) {
    return nullptr; // Stub
}

} // namespace hotc
