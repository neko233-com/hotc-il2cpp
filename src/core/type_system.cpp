#include "type_system.h"

namespace hotc {

TypeSystem::TypeSystem() {
    object_type_ = RegisterType("System.Object", TypeKind::Object, 8);
    int32_type_ = RegisterType("System.Int32", TypeKind::Int32, 4);
    float_type_ = RegisterType("System.Single", TypeKind::Single, 4);
    string_type_ = RegisterType("System.String", TypeKind::String, 8);

    RegisterType("System.Void", TypeKind::Void, 0);
    RegisterType("System.Boolean", TypeKind::Boolean, 1);
    RegisterType("System.Char", TypeKind::Char, 2);
    RegisterType("System.SByte", TypeKind::SByte, 1);
    RegisterType("System.Byte", TypeKind::Byte, 1);
    RegisterType("System.Int16", TypeKind::Int16, 2);
    RegisterType("System.UInt16", TypeKind::UInt16, 2);
    RegisterType("System.UInt32", TypeKind::UInt32, 4);
    RegisterType("System.Int64", TypeKind::Int64, 8);
    RegisterType("System.UInt64", TypeKind::UInt64, 8);
    RegisterType("System.Double", TypeKind::Double, 8);
}

TypeInfo* TypeSystem::GetType(const std::string& name) {
    auto it = types_.find(name);
    if (it != types_.end()) {
        return it->second.get();
    }
    return nullptr;
}

TypeInfo* TypeSystem::RegisterType(const std::string& name, TypeKind kind, uint32_t size) {
    auto type = std::make_unique<TypeInfo>();
    type->name = name;
    type->kind = kind;
    type->size = size;

    TypeInfo* ptr = type.get();
    types_[name] = std::move(type);
    return ptr;
}

TypeInfo* TypeSystem::RegisterClassType(const std::string& name, TypeInfo* base_type) {
    TypeInfo* type = RegisterType(name, TypeKind::Class, 8);
    if (base_type) {
        type->base_type = base_type;
    }
    return type;
}

TypeInfo* TypeSystem::RegisterStructType(const std::string& name, uint32_t size) {
    return RegisterType(name, TypeKind::Struct, size);
}

void TypeSystem::RegisterField(TypeInfo* type, const std::string& field_name, TypeInfo* field_type, uint32_t offset, bool is_static) {
    FieldInfo field;
    field.name = field_name;
    field.type = field_type;
    field.offset = offset;
    field.is_static = is_static;

    std::string key = type->name + "::" + field_name;
    fields_[key].push_back(field);
    type->fields.push_back(field_type);
}

void TypeSystem::RegisterMethod(TypeInfo* type, const MethodInfo& method) {
    std::string key = type->name + "::" + method.name;
    methods_[key].push_back(method);
}

MethodInfo* TypeSystem::GetMethod(TypeInfo* type, const std::string& name) {
    std::string key = type->name + "::" + name;
    auto it = methods_.find(key);
    if (it != methods_.end() && !it->second.empty()) {
        return &it->second[0];
    }

    if (type->base_type) {
        return GetMethod(type->base_type, name);
    }

    return nullptr;
}

FieldInfo* TypeSystem::GetField(TypeInfo* type, const std::string& name) {
    std::string key = type->name + "::" + name;
    auto it = fields_.find(key);
    if (it != fields_.end() && !it->second.empty()) {
        return &it->second[0];
    }

    if (type->base_type) {
        return GetField(type->base_type, name);
    }

    return nullptr;
}

} // namespace hotc
