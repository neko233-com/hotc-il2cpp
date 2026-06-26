#pragma once

#include "platform.h"
#include "metadata.h"
#include "hp_memory.h"
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <memory>
#include <string>
#include <functional>

namespace hotc {

// Complete IL2CPP metadata loader
class IL2CPPMetadataLoader {
public:
    IL2CPPMetadataLoader() = default;
    ~IL2CPPMetadataLoader() = default;
    
    // Load global-metadata.dat
    bool LoadGlobalMetadata(const uint8_t* data, size_t size);
    bool LoadGlobalMetadataFromFile(const std::string& path);
    
    // Load assembly image
    bool LoadAssemblyImage(const uint8_t* data, size_t size, const std::string& name);
    bool LoadAssemblyImageFromFile(const std::string& path);
    
    // Type resolution
    uint32_t GetTypeIndex(const char* full_name) const;
    uint32_t GetTypeIndex(const char* name, const char* namespace_name) const;
    uint32_t GetTypeIndexFromType(const Il2CppTypeDefinition* type_def) const;
    
    // Method resolution
    uint32_t GetMethodIndex(uint32_t type_index, const char* method_name, uint32_t param_count = 0) const;
    uint32_t GetMethodIndexFromDef(const Il2CppMethodDefinition* method_def) const;
    
    // Field resolution
    uint32_t GetFieldIndex(uint32_t type_index, const char* field_name) const;
    
    // Property resolution
    uint32_t GetPropertyIndex(uint32_t type_index, const char* property_name) const;
    
    // Generic instantiation
    uint32_t InstantiateGenericMethod(uint32_t method_index, const std::vector<uint32_t>& type_args);
    uint32_t InstantiateGenericType(uint32_t type_index, const std::vector<uint32_t>& type_args);
    
    // Runtime type creation
    struct RuntimeType {
        uint32_t index;
        std::string name;
        std::string namespace_name;
        uint32_t parent_index;
        uint32_t element_type_index;
        uint32_t generic_inst_index;
        uint32_t flags;
        uint32_t instance_size;
        uint32_t field_count;
        uint32_t method_count;
        uint32_t vtable_count;
        uint32_t interface_count;
        
        // Cached data
        std::vector<uint32_t> field_indices;
        std::vector<uint32_t> method_indices;
        std::vector<uint32_t> interface_indices;
    };
    
    // Runtime method creation
    struct RuntimeMethod {
        uint32_t index;
        std::string name;
        uint32_t declaring_type;
        uint32_t return_type;
        uint32_t parameter_count;
        std::vector<uint32_t> parameter_types;
        uint32_t flags;
        uint32_t token;
        uint32_t vtable_index;
        uint32_t slot;
        
        // Native bridge
        void* native_func;
        void* invoker_func;
    };
    
    const RuntimeType* GetRuntimeType(uint32_t index) const;
    const RuntimeMethod* GetRuntimeMethod(uint32_t index) const;
    
    // Metadata access
    const Il2CppTypeDefinition* GetTypeDefinition(uint32_t index) const;
    const Il2CppMethodDefinition* GetMethodDefinition(uint32_t index) const;
    const Il2CppFieldDefinition* GetFieldDefinition(uint32_t index) const;
    const Il2CppImageDefinition* GetImageDefinition(uint32_t index) const;
    const Il2CppAssemblyDefinition* GetAssemblyDefinition(uint32_t index) const;
    
    const char* GetString(uint32_t index) const;
    
    // Statistics
    uint32_t GetTypeCount() const { return runtime_types_.size(); }
    uint32_t GetMethodCount() const { return runtime_methods_.size(); }
    uint32_t GetImageCount() const { return images_.size(); }
    
private:
    bool ParseGlobalMetadata(const uint8_t* data, size_t size);
    bool ValidateMetadataHeader(const Il2CppGlobalMetadataHeader* header);
    void BuildTypeCaches();
    
    // Raw metadata
    std::vector<uint8_t> metadata_data_;
    const Il2CppGlobalMetadataHeader* header_ = nullptr;
    
    // Type definitions
    std::vector<Il2CppTypeDefinition> type_definitions_;
    std::vector<Il2CppMethodDefinition> method_definitions_;
    std::vector<Il2CppFieldDefinition> field_definitions_;
    std::vector<Il2CppImageDefinition> images_;
    std::vector<Il2CppAssemblyDefinition> assemblies_;
    
    // String table
    std::vector<uint8_t> string_table_;
    
    // Runtime data
    std::vector<RuntimeType> runtime_types_;
    std::vector<RuntimeMethod> runtime_methods_;
    
    // Fast lookup caches
    std::unordered_map<uint64_t, uint32_t> type_name_cache_;
    std::unordered_map<uint64_t, uint32_t> method_name_cache_;
    std::unordered_map<uint64_t, uint32_t> field_name_cache_;
    
    // Generic instantiation cache
    std::unordered_map<uint64_t, uint32_t> generic_method_cache_;
    std::unordered_map<uint64_t, uint32_t> generic_type_cache_;
    
    // Assembly images
    struct AssemblyImage {
        std::string name;
        std::vector<uint8_t> data;
        std::vector<Il2CppTypeDefinition> types;
        std::vector<Il2CppMethodDefinition> methods;
    };
    
    std::vector<AssemblyImage> assembly_images_;
    
    bool loaded_ = false;
};

} // namespace hotc
