#pragma once

#include "../platform.h"
#include "../core/hp_interpreter.h"
#include "../core/il2cpp_metadata.h"
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <string>
#include <functional>

namespace hotc {

// Unity engine types
struct UnityEngine_Vector3 {
    float x, y, z;
    
    UnityEngine_Vector3() : x(0), y(0), z(0) {}
    UnityEngine_Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
    
    UnityEngine_Vector3 operator+(const UnityEngine_Vector3& other) const {
        return {x + other.x, y + other.y, z + other.z};
    }
    
    UnityEngine_Vector3 operator-(const UnityEngine_Vector3& other) const {
        return {x - other.x, y - other.y, z - other.z};
    }
    
    UnityEngine_Vector3 operator*(float s) const {
        return {x * s, y * s, z * s};
    }
    
    float Dot(const UnityEngine_Vector3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }
    
    UnityEngine_Vector3 Cross(const UnityEngine_Vector3& other) const {
        return {
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        };
    }
    
    float Magnitude() const {
        return std::sqrt(x * x + y * y + z * z);
    }
    
    UnityEngine_Vector3 Normalized() const {
        float m = Magnitude();
        if (m > 0) return *this * (1.0f / m);
        return {0, 0, 0};
    }
};

struct UnityEngine_Quaternion {
    float x, y, z, w;
    
    UnityEngine_Quaternion() : x(0), y(0), z(0), w(1) {}
    UnityEngine_Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    
    static UnityEngine_Quaternion Euler(float roll, float pitch, float yaw) {
        float cr = std::cos(roll * 0.5f);
        float sr = std::sin(roll * 0.5f);
        float cp = std::cos(pitch * 0.5f);
        float sp = std::sin(pitch * 0.5f);
        float cy = std::cos(yaw * 0.5f);
        float sy = std::sin(yaw * 0.5f);
        
        return {
            sr * cp * cy - cr * sp * sy,
            cr * sp * cy + sr * cp * sy,
            cr * cp * sy - sr * sp * cy,
            cr * cp * cy + sr * sp * sy
        };
    }
};

struct UnityEngine_Color {
    float r, g, b, a;
    
    UnityEngine_Color() : r(0), g(0), b(0), a(1) {}
    UnityEngine_Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
    
    static UnityEngine_Color Red() { return {1, 0, 0, 1}; }
    static UnityEngine_Color Green() { return {0, 1, 0, 1}; }
    static UnityEngine_Color Blue() { return {0, 0, 1, 1}; }
    static UnityEngine_Color White() { return {1, 1, 1, 1}; }
    static UnityEngine_Color Black() { return {0, 0, 0, 1}; }
};

struct UnityEngine_Rect {
    float x, y, width, height;
    
    UnityEngine_Rect() : x(0), y(0), width(0), height(0) {}
    UnityEngine_Rect(float x, float y, float w, float h) : x(x), y(y), width(w), height(h) {}
    
    bool Contains(float px, float py) const {
        return px >= x && px <= x + width && py >= y && py <= y + height;
    }
};

// Unity Object hierarchy
struct UnityEngine_Object {
    uint32_t instance_id;
    uint32_t native_ptr;
    bool is_active;
    
    virtual ~UnityEngine_Object() = default;
};

struct UnityEngine_Component : UnityEngine_Object {
    uint32_t game_object_id;
    
    virtual ~UnityEngine_Component() = default;
};

struct UnityEngine_Behaviour : UnityEngine_Component {
    bool enabled;
    
    virtual ~UnityEngine_Behaviour() = default;
    virtual void Update() {}
    virtual void Start() {}
    virtual void Awake() {}
    virtual void OnDestroy() {}
};

struct UnityEngine_MonoBehaviour : UnityEngine_Behaviour {
    virtual ~UnityEngine_MonoBehaviour() = default;
};

struct UnityEngine_Transform : UnityEngine_Component {
    UnityEngine_Vector3 local_position;
    UnityEngine_Quaternion local_rotation;
    UnityEngine_Vector3 local_scale;
    uint32_t parent_id;
    std::vector<uint32_t> children;
    
    UnityEngine_Vector3 GetPosition() const { return local_position; }
    void SetPosition(const UnityEngine_Vector3& pos) { local_position = pos; }
    UnityEngine_Quaternion GetRotation() const { return local_rotation; }
    void SetRotation(const UnityEngine_Quaternion& rot) { local_rotation = rot; }
    UnityEngine_Vector3 GetScale() const { return local_scale; }
    void SetScale(const UnityEngine_Vector3& s) { local_scale = s; }
    void SetParent(uint32_t parent) { parent_id = parent; }
    void Translate(const UnityEngine_Vector3& translation) { local_position = local_position + translation; }
    void Rotate(float x, float y, float z) { local_rotation = UnityEngine_Quaternion::Euler(x, y, z); }
};

struct UnityEngine_GameObject : UnityEngine_Object {
    std::string name;
    uint32_t transform_id;
    std::vector<uint32_t> components;
    bool active_self;
    int32_t layer;
    
    UnityEngine_GameObject() : active_self(true), layer(0) {}
    
    UnityEngine_Transform* GetTransform() const;
    void SetActive(bool active) { active_self = active; }
    bool GetActive() const { return active_self; }
    void SetLayer(int32_t l) { layer = l; }
    int32_t GetLayer() const { return layer; }
};

struct UnityEngine_Rigidbody : UnityEngine_Component {
    UnityEngine_Vector3 velocity;
    UnityEngine_Vector3 angular_velocity;
    float mass;
    float drag;
    float angular_drag;
    bool use_gravity;
    bool is_kinematic;
    
    UnityEngine_Rigidbody() : mass(1.0f), drag(0), angular_drag(0.05f), 
                              use_gravity(true), is_kinematic(false) {}
    
    void AddForce(const UnityEngine_Vector3& force) { velocity = velocity + force * (1.0f / mass); }
    void AddTorque(const UnityEngine_Vector3& torque) { angular_velocity = angular_velocity + torque * (1.0f / mass); }
    void SetVelocity(const UnityEngine_Vector3& v) { velocity = v; }
    UnityEngine_Vector3 GetVelocity() const { return velocity; }
};

struct UnityEngine_Collider : UnityEngine_Component {
    bool enabled;
    bool is_trigger;
    
    UnityEngine_Collider() : enabled(true), is_trigger(false) {}
};

struct UnityEngine_BoxCollider : UnityEngine_Collider {
    UnityEngine_Vector3 center;
    UnityEngine_Vector3 size;
    
    UnityEngine_BoxCollider() : size({1, 1, 1}) {}
};

struct UnityEngine_SphereCollider : UnityEngine_Collider {
    UnityEngine_Vector3 center;
    float radius;
    
    UnityEngine_SphereCollider() : radius(0.5f) {}
};

struct UnityEngine_Camera : UnityEngine_Behaviour {
    float field_of_view;
    float near_clip;
    float far_clip;
    UnityEngine_Color background_color;
    int32_t culling_mask;
    
    UnityEngine_Camera() : field_of_view(60.0f), near_clip(0.3f), far_clip(1000.0f), culling_mask(-1) {}
};

struct UnityEngine_Light : UnityEngine_Behaviour {
    int32_t type;
    UnityEngine_Color color;
    float intensity;
    float range;
    float spot_angle;
    
    UnityEngine_Light() : type(1), intensity(1.0f), range(10.0f), spot_angle(30.0f) {}
};

struct UnityEngine_Renderer : UnityEngine_Component {
    bool enabled;
    int32_t sorting_layer_id;
    int32_t sorting_order;
    
    UnityEngine_Renderer() : enabled(true), sorting_layer_id(0), sorting_order(0) {}
};

struct UnityEngine_MeshRenderer : UnityEngine_Renderer {
    uint32_t material_id;
    uint32_t mesh_id;
};

struct UnityEngine_Animator : UnityEngine_Behaviour {
    float speed;
    bool apply_root_motion;
    
    UnityEngine_Animator() : speed(1.0f), apply_root_motion(false) {}
    
    void SetFloat(const char* name, float value) {}
    void SetBool(const char* name, bool value) {}
    void SetInteger(const char* name, int32_t value) {}
    void SetTrigger(const char* name) {}
};

struct UnityEngine_AudioSource : UnityEngine_Behaviour {
    uint32_t clip_id;
    float volume;
    float pitch;
    bool loop;
    bool play_on_awake;
    
    UnityEngine_AudioSource() : volume(1.0f), pitch(1.0f), loop(false), play_on_awake(true) {}
    
    void Play() {}
    void Stop() {}
    void Pause() {}
};

// Unity API Bridge
class UnityBridge {
public:
    UnityBridge(HighPerformanceInterpreter& interpreter, IL2CPPMetadataLoader& metadata);
    ~UnityBridge() = default;
    
    // Register all Unity API methods
    void RegisterAll();
    
    // Object management
    uint32_t CreateGameObject(const char* name);
    void DestroyGameObject(uint32_t id);
    UnityEngine_GameObject* GetGameObject(uint32_t id);
    
    // Component management
    uint32_t AddComponent(uint32_t game_object_id, uint32_t type_index);
    void* GetComponent(uint32_t game_object_id, uint32_t type_index);
    
    // Transform operations
    void SetPosition(uint32_t transform_id, float x, float y, float z);
    void GetPosition(uint32_t transform_id, float& x, float& y, float& z);
    void SetRotation(uint32_t transform_id, float x, float y, float z, float w);
    void GetRotation(uint32_t transform_id, float& x, float& y, float& z, float& w);
    void SetScale(uint32_t transform_id, float x, float y, float z);
    void GetScale(uint32_t transform_id, float& x, float& y, float& z);
    
    // Physics
    bool Raycast(UnityEngine_Vector3 origin, UnityEngine_Vector3 direction, float max_distance, 
                 uint32_t layer_mask, uint32_t& hit_game_object, UnityEngine_Vector3& hit_point, 
                 UnityEngine_Vector3& hit_normal);
    
    // Input
    float GetAxisRaw(const char* axis_name);
    bool GetButton(const char* button_name);
    bool GetButtonDown(const char* button_name);
    bool GetButtonUp(const char* button_name);
    
    // Time
    float GetDeltaTime();
    float GetTime();
    float GetTimeScale();
    void SetTimeScale(float scale);
    
    // Debug
    void Log(const char* message);
    void LogWarning(const char* message);
    void LogError(const char* message);
    
    // Performance
    uint32_t GetObjectCount() const;
    uint64_t GetTotalMemoryUsed() const;
    
private:
    HighPerformanceInterpreter& interpreter_;
    IL2CPPMetadataLoader& metadata_;
    
    // Object storage
    std::unordered_map<uint32_t, std::unique_ptr<UnityEngine_GameObject>> game_objects_;
    std::unordered_map<uint32_t, std::unique_ptr<UnityEngine_Transform>> transforms_;
    std::unordered_map<uint32_t, std::unique_ptr<UnityEngine_Component>> components_;
    
    uint32_t next_id_ = 1;
    
    // Register native functions
    void RegisterGameObjectFunctions();
    void RegisterTransformFunctions();
    void RegisterComponentFunctions();
    void RegisterPhysicsFunctions();
    void RegisterInputFunctions();
    void RegisterTimeFunctions();
    void RegisterDebugFunctions();
};

} // namespace hotc
