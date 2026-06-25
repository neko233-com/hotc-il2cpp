#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace hotc {

enum class TypeKind {
    Void,
    Boolean,
    Char,
    SByte,
    Byte,
    Int16,
    UInt16,
    Int32,
    UInt32,
    Int64,
    UInt64,
    Single,
    Double,
    String,
    Object,
    Class,
    Struct,
    Enum,
    Array,
    Pointer,
    Reference,
};

struct TypeInfo {
    std::string name;
    TypeKind kind;
    uint32_t size;
    TypeInfo* element_type = nullptr;
    TypeInfo* base_type = nullptr;
    std::vector<TypeInfo*> fields;
    std::vector<TypeInfo*> interfaces;
};

struct FieldInfo {
    std::string name;
    TypeInfo* type;
    uint32_t offset;
    bool is_static;
};

struct MethodInfo {
    std::string name;
    TypeInfo* return_type;
    std::vector<TypeInfo*> parameter_types;
    void* native_func = nullptr;
    bool is_virtual;
    bool is_static;
};

class TypeSystem {
public:
    TypeSystem();
    ~TypeSystem() = default;

    TypeInfo* GetType(const std::string& name);
    TypeInfo* RegisterType(const std::string& name, TypeKind kind, uint32_t size);
    TypeInfo* RegisterClassType(const std::string& name, TypeInfo* base_type = nullptr);
    TypeInfo* RegisterStructType(const std::string& name, uint32_t size);

    void RegisterField(TypeInfo* type, const std::string& field_name, TypeInfo* field_type, uint32_t offset, bool is_static = false);
    void RegisterMethod(TypeInfo* type, const MethodInfo& method);

    MethodInfo* GetMethod(TypeInfo* type, const std::string& name);
    FieldInfo* GetField(TypeInfo* type, const std::string& name);

    TypeInfo* GetObjectType() const { return object_type_; }
    TypeInfo* GetInt32Type() const { return int32_type_; }
    TypeInfo* GetFloatType() const { return float_type_; }
    TypeInfo* GetStringType() const { return string_type_; }

private:
    std::unordered_map<std::string, std::unique_ptr<TypeInfo>> types_;
    std::unordered_map<std::string, std::vector<MethodInfo>> methods_;
    std::unordered_map<std::string, std::vector<FieldInfo>> fields_;

    TypeInfo* object_type_ = nullptr;
    TypeInfo* int32_type_ = nullptr;
    TypeInfo* float_type_ = nullptr;
    TypeInfo* string_type_ = nullptr;
};

} // namespace hotc
