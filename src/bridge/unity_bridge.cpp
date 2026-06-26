#include "unity_bridge.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <chrono>

namespace hotc {

UnityBridge::UnityBridge(HighPerformanceInterpreter& interpreter, IL2CPPMetadataLoader& metadata)
    : interpreter_(interpreter), metadata_(metadata) {
}

void UnityBridge::RegisterAll() {
    RegisterGameObjectFunctions();
    RegisterTransformFunctions();
    RegisterComponentFunctions();
    RegisterPhysicsFunctions();
    RegisterInputFunctions();
    RegisterTimeFunctions();
    RegisterDebugFunctions();
}

void UnityBridge::RegisterGameObjectFunctions() {
    interpreter_.RegisterNativeFunction("UnityEngine.GameObject::Create", 
        [this](void** args, uint32_t arg_count) -> void* {
            const char* name = static_cast<const char*>(args[0]);
            uint32_t id = CreateGameObject(name);
            return reinterpret_cast<void*>(static_cast<uintptr_t>(id));
        });
    
    interpreter_.RegisterNativeFunction("UnityEngine.GameObject::Find", 
        [this](void** args, uint32_t arg_count) -> void* {
            const char* name = static_cast<const char*>(args[0]);
            for (auto& [id, obj] : game_objects_) {
                if (obj->name == name) {
                    return reinterpret_cast<void*>(static_cast<uintptr_t>(id));
                }
            }
            return nullptr;
        });
    
    interpreter_.RegisterNativeFunction("UnityEngine.GameObject::SetActive", 
        [this](void** args, uint32_t arg_count) -> void* {
            uint32_t id = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(args[0]));
            bool active = static_cast<bool>(reinterpret_cast<uintptr_t>(args[1]));
            auto* obj = GetGameObject(id);
            if (obj) obj->SetActive(active);
            return nullptr;
        });
    
    interpreter_.RegisterNativeFunction("UnityEngine.GameObject::GetActive", 
        [this](void** args, uint32_t arg_count) -> void* {
            uint32_t id = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(args[0]));
            auto* obj = GetGameObject(id);
            return reinterpret_cast<void*>(static_cast<uintptr_t>(obj ? obj->GetActive() : false));
        });
    
    interpreter_.RegisterNativeFunction("UnityEngine.GameObject::Destroy", 
        [this](void** args, uint32_t arg_count) -> void* {
            uint32_t id = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(args[0]));
            DestroyGameObject(id);
            return nullptr;
        });
    
    interpreter_.RegisterNativeFunction("UnityEngine.GameObject::AddComponent", 
        [this](void** args, uint32_t arg_count) -> void* {
            uint32_t go_id = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(args[0]));
            uint32_t type_index = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(args[1]));
            uint32_t comp_id = AddComponent(go_id, type_index);
            return reinterpret_cast<void*>(static_cast<uintptr_t>(comp_id));
        });
}

void UnityBridge::RegisterTransformFunctions() {
    interpreter_.RegisterNativeFunction("UnityEngine.Transform::SetPosition", 
        [this](void** args, uint32_t arg_count) -> void* {
            uint32_t id = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(args[0]));
            float x = *static_cast<float*>(args[1]);
            float y = *static_cast<float*>(args[2]);
            float z = *static_cast<float*>(args[3]);
            SetPosition(id, x, y, z);
            return nullptr;
        });
    
    interpreter_.RegisterNativeFunction("UnityEngine.Transform::GetPosition", 
        [this](void** args, uint32_t arg_count) -> void* {
            uint32_t id = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(args[0]));
            float x, y, z;
            GetPosition(id, x, y, z);
            float* result = new float[3]{x, y, z};
            return result;
        });
    
    interpreter_.RegisterNativeFunction("UnityEngine.Transform::SetRotation", 
        [this](void** args, uint32_t arg_count) -> void* {
            uint32_t id = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(args[0]));
            float x = *static_cast<float*>(args[1]);
            float y = *static_cast<float*>(args[2]);
            float z = *static_cast<float*>(args[3]);
            float w = *static_cast<float*>(args[4]);
            SetRotation(id, x, y, z, w);
            return nullptr;
        });
    
    interpreter_.RegisterNativeFunction("UnityEngine.Transform::SetScale", 
        [this](void** args, uint32_t arg_count) -> void* {
            uint32_t id = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(args[0]));
            float x = *static_cast<float*>(args[1]);
            float y = *static_cast<float*>(args[2]);
            float z = *static_cast<float*>(args[3]);
            SetScale(id, x, y, z);
            return nullptr;
        });
    
    interpreter_.RegisterNativeFunction("UnityEngine.Transform::Translate", 
        [this](void** args, uint32_t arg_count) -> void* {
            uint32_t id = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(args[0]));
            float x = *static_cast<float*>(args[1]);
            float y = *static_cast<float*>(args[2]);
            float z = *static_cast<float*>(args[3]);
            auto* transform = transforms_[id].get();
            if (transform) {
                transform->Translate({x, y, z});
            }
            return nullptr;
        });
}

void UnityBridge::RegisterComponentFunctions() {
    interpreter_.RegisterNativeFunction("UnityEngine.Component::GetGameObject", 
        [this](void** args, uint32_t arg_count) -> void* {
            uint32_t comp_id = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(args[0]));
            auto* comp = components_[comp_id].get();
            if (comp) {
                return reinterpret_cast<void*>(static_cast<uintptr_t>(comp->game_object_id));
            }
            return nullptr;
        });
    
    interpreter_.RegisterNativeFunction("UnityEngine.Component::GetTransform", 
        [this](void** args, uint32_t arg_count) -> void* {
            uint32_t comp_id = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(args[0]));
            auto* comp = components_[comp_id].get();
            if (comp) {
                auto* go = GetGameObject(comp->game_object_id);
                if (go) {
                    return reinterpret_cast<void*>(static_cast<uintptr_t>(go->transform_id));
                }
            }
            return nullptr;
        });
}

void UnityBridge::RegisterPhysicsFunctions() {
    interpreter_.RegisterNativeFunction("UnityEngine.Physics::Raycast", 
        [this](void** args, uint32_t arg_count) -> void* {
            float ox = *static_cast<float*>(args[0]);
            float oy = *static_cast<float*>(args[1]);
            float oz = *static_cast<float*>(args[2]);
            float dx = *static_cast<float*>(args[3]);
            float dy = *static_cast<float*>(args[4]);
            float dz = *static_cast<float*>(args[5]);
            float max_dist = *static_cast<float*>(args[6]);
            
            uint32_t hit_id;
            UnityEngine_Vector3 hit_point, hit_normal;
            bool hit = Raycast({ox, oy, oz}, {dx, dy, dz}, max_dist, -1, hit_id, hit_point, hit_normal);
            return reinterpret_cast<void*>(static_cast<uintptr_t>(hit));
        });
}

void UnityBridge::RegisterInputFunctions() {
    interpreter_.RegisterNativeFunction("UnityEngine.Input::GetAxisRaw", 
        [this](void** args, uint32_t arg_count) -> void* {
            const char* axis = static_cast<const char*>(args[0]);
            float value = GetAxisRaw(axis);
            return reinterpret_cast<void*>(static_cast<uintptr_t>(*reinterpret_cast<uint32_t*>(&value)));
        });
    
    interpreter_.RegisterNativeFunction("UnityEngine.Input::GetButton", 
        [this](void** args, uint32_t arg_count) -> void* {
            const char* button = static_cast<const char*>(args[0]);
            return reinterpret_cast<void*>(static_cast<uintptr_t>(GetButton(button)));
        });
}

void UnityBridge::RegisterTimeFunctions() {
    interpreter_.RegisterNativeFunction("UnityEngine.Time::GetDeltaTime", 
        [this](void** args, uint32_t arg_count) -> void* {
            float dt = GetDeltaTime();
            return reinterpret_cast<void*>(static_cast<uintptr_t>(*reinterpret_cast<uint32_t*>(&dt)));
        });
    
    interpreter_.RegisterNativeFunction("UnityEngine.Time::GetTime", 
        [this](void** args, uint32_t arg_count) -> void* {
            float t = GetTime();
            return reinterpret_cast<void*>(static_cast<uintptr_t>(*reinterpret_cast<uint32_t*>(&t)));
        });
    
    interpreter_.RegisterNativeFunction("UnityEngine.Time::GetTimeScale", 
        [this](void** args, uint32_t arg_count) -> void* {
            float s = GetTimeScale();
            return reinterpret_cast<void*>(static_cast<uint32_t>(*reinterpret_cast<uint32_t*>(&s)));
        });
    
    interpreter_.RegisterNativeFunction("UnityEngine.Time::SetTimeScale", 
        [this](void** args, uint32_t arg_count) -> void* {
            float s = *static_cast<float*>(args[0]);
            SetTimeScale(s);
            return nullptr;
        });
}

void UnityBridge::RegisterDebugFunctions() {
    interpreter_.RegisterNativeFunction("UnityEngine.Debug::Log", 
        [this](void** args, uint32_t arg_count) -> void* {
            const char* msg = static_cast<const char*>(args[0]);
            Log(msg);
            return nullptr;
        });
    
    interpreter_.RegisterNativeFunction("UnityEngine.Debug::LogWarning", 
        [this](void** args, uint32_t arg_count) -> void* {
            const char* msg = static_cast<const char*>(args[0]);
            LogWarning(msg);
            return nullptr;
        });
    
    interpreter_.RegisterNativeFunction("UnityEngine.Debug::LogError", 
        [this](void** args, uint32_t arg_count) -> void* {
            const char* msg = static_cast<const char*>(args[0]);
            LogError(msg);
            return nullptr;
        });
}

uint32_t UnityBridge::CreateGameObject(const char* name) {
    uint32_t id = next_id_++;
    
    auto go = std::make_unique<UnityEngine_GameObject>();
    go->instance_id = id;
    go->name = name;
    go->transform_id = next_id_++;
    go->active_self = true;
    
    auto transform = std::make_unique<UnityEngine_Transform>();
    transform->instance_id = go->transform_id;
    transform->game_object_id = id;
    
    game_objects_[id] = std::move(go);
    transforms_[go->transform_id] = std::move(transform);
    
    return id;
}

void UnityBridge::DestroyGameObject(uint32_t id) {
    auto it = game_objects_.find(id);
    if (it != game_objects_.end()) {
        // Remove components
        for (uint32_t comp_id : it->second->components) {
            components_.erase(comp_id);
        }
        
        // Remove transform
        transforms_.erase(it->second->transform_id);
        
        game_objects_.erase(it);
    }
}

UnityEngine_GameObject* UnityBridge::GetGameObject(uint32_t id) {
    auto it = game_objects_.find(id);
    return (it != game_objects_.end()) ? it->second.get() : nullptr;
}

uint32_t UnityBridge::AddComponent(uint32_t game_object_id, uint32_t type_index) {
    auto* go = GetGameObject(game_object_id);
    if (!go) return 0;
    
    uint32_t comp_id = next_id_++;
    auto comp = std::make_unique<UnityEngine_Component>();
    comp->instance_id = comp_id;
    comp->game_object_id = game_object_id;
    
    go->components.push_back(comp_id);
    components_[comp_id] = std::move(comp);
    
    return comp_id;
}

void* UnityBridge::GetComponent(uint32_t game_object_id, uint32_t type_index) {
    auto* go = GetGameObject(game_object_id);
    if (!go) return nullptr;
    
    for (uint32_t comp_id : go->components) {
        auto* comp = components_[comp_id].get();
        if (comp && comp->instance_id == type_index) {
            return comp;
        }
    }
    return nullptr;
}

void UnityBridge::SetPosition(uint32_t transform_id, float x, float y, float z) {
    auto it = transforms_.find(transform_id);
    if (it != transforms_.end()) {
        it->second->SetPosition({x, y, z});
    }
}

void UnityBridge::GetPosition(uint32_t transform_id, float& x, float& y, float& z) {
    auto it = transforms_.find(transform_id);
    if (it != transforms_.end()) {
        auto pos = it->second->GetPosition();
        x = pos.x;
        y = pos.y;
        z = pos.z;
    }
}

void UnityBridge::SetRotation(uint32_t transform_id, float x, float y, float z, float w) {
    auto it = transforms_.find(transform_id);
    if (it != transforms_.end()) {
        it->second->SetRotation({x, y, z, w});
    }
}

void UnityBridge::GetRotation(uint32_t transform_id, float& x, float& y, float& z, float& w) {
    auto it = transforms_.find(transform_id);
    if (it != transforms_.end()) {
        auto rot = it->second->GetRotation();
        x = rot.x;
        y = rot.y;
        z = rot.z;
        w = rot.w;
    }
}

void UnityBridge::SetScale(uint32_t transform_id, float x, float y, float z) {
    auto it = transforms_.find(transform_id);
    if (it != transforms_.end()) {
        it->second->SetScale({x, y, z});
    }
}

void UnityBridge::GetScale(uint32_t transform_id, float& x, float& y, float& z) {
    auto it = transforms_.find(transform_id);
    if (it != transforms_.end()) {
        auto s = it->second->GetScale();
        x = s.x;
        y = s.y;
        z = s.z;
    }
}

bool UnityBridge::Raycast(UnityEngine_Vector3 origin, UnityEngine_Vector3 direction, float max_distance,
                          uint32_t layer_mask, uint32_t& hit_game_object, UnityEngine_Vector3& hit_point,
                          UnityEngine_Vector3& hit_normal) {
    // Simple raycast against all colliders
    float closest_t = max_distance;
    bool hit = false;
    
    for (auto& [id, go] : game_objects_) {
        if (!go->active_self) continue;
        
        auto* transform = transforms_[go->transform_id].get();
        if (!transform) continue;
        
        // Simple AABB test (would need proper collider shapes in real implementation)
        auto pos = transform->GetPosition();
        float dx = origin.x - pos.x;
        float dy = origin.y - pos.y;
        float dz = origin.z - pos.z;
        
        float t = -(dx * direction.x + dy * direction.y + dz * direction.z) /
                  (direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
        
        if (t > 0 && t < closest_t) {
            closest_t = t;
            hit_game_object = id;
            hit_point = origin + direction * t;
            hit_normal = {0, 1, 0};
            hit = true;
        }
    }
    
    return hit;
}

float UnityBridge::GetAxisRaw(const char* axis_name) {
    // Stub - would integrate with actual input system
    return 0.0f;
}

bool UnityBridge::GetButton(const char* button_name) {
    return false;
}

bool UnityBridge::GetButtonDown(const char* button_name) {
    return false;
}

bool UnityBridge::GetButtonUp(const char* button_name) {
    return false;
}

float UnityBridge::GetDeltaTime() {
    static auto last_time = std::chrono::high_resolution_clock::now();
    auto now = std::chrono::high_resolution_clock::now();
    float dt = std::chrono::duration<float>(now - last_time).count();
    last_time = now;
    return dt;
}

float UnityBridge::GetTime() {
    static auto start_time = std::chrono::high_resolution_clock::now();
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<float>(now - start_time).count();
}

float UnityBridge::GetTimeScale() {
    return 1.0f;
}

void UnityBridge::SetTimeScale(float scale) {
    // Stub
}

void UnityBridge::Log(const char* message) {
    printf("[LOG] %s\n", message);
}

void UnityBridge::LogWarning(const char* message) {
    printf("[WARNING] %s\n", message);
}

void UnityBridge::LogError(const char* message) {
    printf("[ERROR] %s\n", message);
}

uint32_t UnityBridge::GetObjectCount() const {
    return static_cast<uint32_t>(game_objects_.size() + transforms_.size() + components_.size());
}

uint64_t UnityBridge::GetTotalMemoryUsed() const {
    uint64_t total = 0;
    for (const auto& [id, obj] : game_objects_) {
        total += sizeof(UnityEngine_GameObject) + obj->name.size();
    }
    for (const auto& [id, transform] : transforms_) {
        total += sizeof(UnityEngine_Transform);
    }
    for (const auto& [id, comp] : components_) {
        total += sizeof(UnityEngine_Component);
    }
    return total;
}

} // namespace hotc
