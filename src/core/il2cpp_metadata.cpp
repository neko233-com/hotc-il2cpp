#include "il2cpp_metadata.h"
#include <fstream>
#include <cstring>

namespace hotc {

bool IL2CPPMetadataLoader::LoadGlobalMetadata(const uint8_t* data, size_t size) {
    if (size < sizeof(Il2CppGlobalMetadataHeader)) return false;
    
    metadata_data_.assign(data, data + size);
    header_ = reinterpret_cast<const Il2CppGlobalMetadataHeader*>(metadata_data_.data());
    
    if (!ValidateMetadataHeader(header_)) return false;
    
    return ParseGlobalMetadata(metadata_data_.data(), metadata_data_.size());
}

bool IL2CPPMetadataLoader::LoadGlobalMetadataFromFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return false;
    
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> buffer(size);
    file.read(reinterpret_cast<char*>(buffer.data()), size);
    
    return LoadGlobalMetadata(buffer.data(), size);
}

bool IL2CPPMetadataLoader::LoadAssemblyImage(const uint8_t* data, size_t size, const std::string& name) {
    AssemblyImage img;
    img.name = name;
    img.data.assign(data, data + size);
    images_.push_back(std::move(img));
    return true;
}

bool IL2CPPMetadataLoader::LoadAssemblyImageFromFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return false;
    
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> buffer(size);
    file.read(reinterpret_cast<char*>(buffer.data()), size);
    
    std::string name = path.substr(path.find_last_of("/\\") + 1);
    return LoadAssemblyImage(buffer.data(), size, name);
}

bool IL2CPPMetadataLoader::ParseGlobalMetadata(const uint8_t* data, size_t size) {
    const Il2CppGlobalMetadataHeader* hdr = reinterpret_cast<const Il2CppGlobalMetadataHeader*>(data);
    
    // Parse type definitions
    const Il2CppTypeDefinition* type_defs = reinterpret_cast<const Il2CppTypeDefinition*>(
        data + hdr->typeDefinitionsOffset);
    type_definitions_.assign(type_defs, type_defs + hdr->typeDefinitionsCount);
    
    // Parse method definitions
    const Il2CppMethodDefinition* method_defs = reinterpret_cast<const Il2CppMethodDefinition*>(
        data + hdr->methodsOffset);
    method_definitions_.assign(method_defs, method_defs + hdr->methodsCount);
    
    // Parse field definitions
    const Il2CppFieldDefinition* field_defs = reinterpret_cast<const Il2CppFieldDefinition*>(
        data + hdr->fieldsOffset);
    field_definitions_.assign(field_defs, field_defs + hdr->fieldsCount);
    
    // Parse images
    const Il2CppImageDefinition* images = reinterpret_cast<const Il2CppImageDefinition*>(
        data + hdr->imagesOffset);
    images_.insert(images_.end(), images, images + hdr->imagesCount);
    
    // Parse assemblies
    const Il2CppAssemblyDefinition* assemblies = reinterpret_cast<const Il2CppAssemblyDefinition*>(
        data + hdr->assembliesOffset);
    assemblies_.assign(assemblies, assemblies + hdr->assembliesCount);
    
    // Store string table
    string_table_.assign(data + hdr->stringOffset, 
                         data + hdr->stringOffset + hdr->stringCount);
    
    // Build runtime types and methods
    BuildTypeCaches();
    
    loaded_ = true;
    return true;
}

bool IL2CPPMetadataLoader::ValidateMetadataHeader(const Il2CppGlobalMetadataHeader* header) {
    // Check magic number (0xFAB11BAF for IL2CPP)
    if (header->magic != 0xFAB11BAF) return false;
    // Check version
    if (header->version < 19 || header->version > 29) return false;
    return true;
}

void IL2CPPMetadataLoader::BuildTypeCaches() {
    runtime_types_.resize(type_definitions_.size());
    
    for (size_t i = 0; i < type_definitions_.size(); i++) {
        const auto& def = type_definitions_[i];
        auto& rt = runtime_types_[i];
        
        rt.index = static_cast<uint32_t>(i);
        rt.name = GetString(def.nameIndex);
        rt.namespace_name = GetString(def.namespaceIndex);
        rt.parent_index = def.parentIndex;
        rt.element_type_index = def.elementTypeIndex;
        rt.flags = def.flags;
        rt.field_count = def.field_count;
        rt.method_count = def.method_count;
        rt.vtable_count = def.vtable_count;
        rt.interface_count = def.interfaces_count;
        
        // Cache type name hash for fast lookup
        uint64_t hash = std::hash<std::string>()(rt.namespace_name + "." + rt.name);
        type_name_cache_[hash] = static_cast<uint32_t>(i);
    }
    
    runtime_methods_.resize(method_definitions_.size());
    
    for (size_t i = 0; i < method_definitions_.size(); i++) {
        const auto& def = method_definitions_[i];
        auto& rm = runtime_methods_[i];
        
        rm.index = static_cast<uint32_t>(i);
        rm.name = GetString(def.nameIndex);
        rm.declaring_type = def.declaringType;
        rm.return_type = def.returnType;
        rm.parameter_count = def.parameterCount;
        rm.flags = def.flags;
        rm.token = def.token;
        
        // Cache method name hash
        uint64_t hash = std::hash<std::string>()(rm.name);
        method_name_cache_[hash] = static_cast<uint32_t>(i);
    }
}

uint32_t IL2CPPMetadataLoader::GetTypeIndex(const char* full_name) const {
    uint64_t hash = std::hash<std::string>()(full_name);
    auto it = type_name_cache_.find(hash);
    return (it != type_name_cache_.end()) ? it->second : UINT32_MAX;
}

uint32_t IL2CPPMetadataLoader::GetTypeIndex(const char* name, const char* namespace_name) const {
    std::string full_name = std::string(namespace_name) + "." + name;
    return GetTypeIndex(full_name.c_str());
}

uint32_t IL2CPPMetadataLoader::GetMethodIndex(uint32_t type_index, const char* method_name, uint32_t param_count) const {
    if (type_index >= runtime_types_.size()) return UINT32_MAX;
    
    const auto& rt = runtime_types_[type_index];
    // Search methods for this type
    for (uint32_t i = 0; i < method_definitions_.size(); i++) {
        const auto& def = method_definitions_[i];
        if (def.declaringType == type_index) {
            const char* name = GetString(def.nameIndex);
            if (std::strcmp(name, method_name) == 0) {
                if (param_count == 0 || def.parameterCount == param_count) {
                    return i;
                }
            }
        }
    }
    return UINT32_MAX;
}

uint32_t IL2CPPMetadataLoader::GetFieldIndex(uint32_t type_index, const char* field_name) const {
    if (type_index >= type_definitions_.size()) return UINT32_MAX;
    
    const auto& def = type_definitions_[type_index];
    uint32_t field_start = def.fieldStart;
    
    for (uint32_t i = 0; i < def.field_count; i++) {
        uint32_t field_idx = field_start + i;
        if (field_idx < field_definitions_.size()) {
            const char* name = GetString(field_definitions_[field_idx].nameIndex);
            if (std::strcmp(name, field_name) == 0) {
                return field_idx;
            }
        }
    }
    return UINT32_MAX;
}

uint32_t IL2CPPMetadataLoader::GetPropertyIndex(uint32_t type_index, const char* property_name) const {
    // Properties are stored similarly to fields
    return UINT32_MAX; // TODO: Implement property resolution
}

uint32_t IL2CPPMetadataLoader::InstantiateGenericMethod(uint32_t method_index, const std::vector<uint32_t>& type_args) {
    // Create hash for generic instantiation
    uint64_t hash = method_index;
    for (auto t : type_args) {
        hash ^= std::hash<uint32_t>()(t) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    }
    
    auto it = generic_method_cache_.find(hash);
    if (it != generic_method_cache_.end()) {
        return it->second;
    }
    
    // Create new generic instantiation
    uint32_t new_index = static_cast<uint32_t>(runtime_methods_.size());
    generic_method_cache_[hash] = new_index;
    
    // Copy base method info
    if (method_index < runtime_methods_.size()) {
        runtime_methods_.push_back(runtime_methods_[method_index]);
        runtime_methods_.back().index = new_index;
    }
    
    return new_index;
}

uint32_t IL2CPPMetadataLoader::InstantiateGenericType(uint32_t type_index, const std::vector<uint32_t>& type_args) {
    uint64_t hash = type_index;
    for (auto t : type_args) {
        hash ^= std::hash<uint32_t>()(t) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    }
    
    auto it = generic_type_cache_.find(hash);
    if (it != generic_type_cache_.end()) {
        return it->second;
    }
    
    uint32_t new_index = static_cast<uint32_t>(runtime_types_.size());
    generic_type_cache_[hash] = new_index;
    
    if (type_index < runtime_types_.size()) {
        runtime_types_.push_back(runtime_types_[type_index]);
        runtime_types_.back().index = new_index;
    }
    
    return new_index;
}

const IL2CPPMetadataLoader::RuntimeType* IL2CPPMetadataLoader::GetRuntimeType(uint32_t index) const {
    return (index < runtime_types_.size()) ? &runtime_types_[index] : nullptr;
}

const IL2CPPMetadataLoader::RuntimeMethod* IL2CPPMetadataLoader::GetRuntimeMethod(uint32_t index) const {
    return (index < runtime_methods_.size()) ? &runtime_methods_[index] : nullptr;
}

const Il2CppTypeDefinition* IL2CPPMetadataLoader::GetTypeDefinition(uint32_t index) const {
    return (index < type_definitions_.size()) ? &type_definitions_[index] : nullptr;
}

const Il2CppMethodDefinition* IL2CPPMetadataLoader::GetMethodDefinition(uint32_t index) const {
    return (index < method_definitions_.size()) ? &method_definitions_[index] : nullptr;
}

const Il2CppFieldDefinition* IL2CPPMetadataLoader::GetFieldDefinition(uint32_t index) const {
    return (index < field_definitions_.size()) ? &field_definitions_[index] : nullptr;
}

const Il2CppImageDefinition* IL2CPPMetadataLoader::GetImageDefinition(uint32_t index) const {
    return (index < images_.size()) ? &images_[index] : nullptr;
}

const Il2CppAssemblyDefinition* IL2CPPMetadataLoader::GetAssemblyDefinition(uint32_t index) const {
    return (index < assemblies_.size()) ? &assemblies_[index] : nullptr;
}

const char* IL2CPPMetadataLoader::GetString(uint32_t index) const {
    if (index < string_table_.size()) {
        return reinterpret_cast<const char*>(string_table_.data() + index);
    }
    return "";
}

} // namespace hotc
