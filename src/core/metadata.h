#pragma once

#include "platform.h"
#include "runtime.h"
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include <cstring>

namespace hotc {

// IL2CPP metadata format structures
// These match Unity's IL2CPP internal structures exactly

#pragma pack(push, 1)

struct Il2CppGlobalMetadataHeader {
    uint32_t magic;
    int32_t version;
    int32_t stringLiteralOffset;
    int32_t stringLiteralCount;
    int32_t stringLiteralDataOffset;
    int32_t stringLiteralDataCount;
    int32_t stringOffset;
    int32_t stringCount;
    int32_t eventsOffset;
    int32_t eventsCount;
    int32_t propertiesOffset;
    int32_t propertiesCount;
    int32_t methodsOffset;
    int32_t methodsCount;
    int32_t parameterDefaultValuesOffset;
    int32_t parameterDefaultValuesCount;
    int32_t fieldDefaultValuesOffset;
    int32_t fieldDefaultValuesCount;
    int32_t fieldAndParameterDefaultValueDataOffset;
    int32_t fieldAndParameterDefaultValueDataCount;
    int32_t fieldMarshaledSizesOffset;
    int32_t fieldMarshaledSizesCount;
    int32_t parametersOffset;
    int32_t parametersCount;
    int32_t fieldsOffset;
    int32_t fieldsCount;
    int32_t genericParametersOffset;
    int32_t genericParametersCount;
    int32_t genericParameterConstraintsOffset;
    int32_t genericParameterConstraintsCount;
    int32_t genericContainersOffset;
    int32_t genericContainersCount;
    int32_t nestedTypesOffset;
    int32_t nestedTypesCount;
    int32_t interfacesOffset;
    int32_t interfacesCount;
    int32_t vtableMethodsOffset;
    int32_t vtableMethodsCount;
    int32_t interfaceOffsetsOffset;
    int32_t interfaceOffsetsCount;
    int32_t typeDefinitionsOffset;
    int32_t typeDefinitionsCount;
    int32_t rgctxEntriesOffset;
    int32_t rgctxEntriesCount;
    int32_t imagesOffset;
    int32_t imagesCount;
    int32_t assembliesOffset;
    int32_t assembliesCount;
    int32_t metadataUsageListsOffset;
    int32_t metadataUsageListsCount;
    int32_t metadataUsagePairsOffset;
    int32_t metadataUsagePairsCount;
    int32_t fieldRefsOffset;
    int32_t fieldRefsCount;
    int32_t referencedAssembliesOffset;
    int32_t referencedAssembliesCount;
    int32_t attributesInfoOffset;
    int32_t attributesInfoCount;
    int32_t attributeTypesOffset;
    int32_t attributeTypesCount;
    int32_t unresolvedVirtualCallParameterTypesOffset;
    int32_t unresolvedVirtualCallParameterTypesCount;
    int32_t unresolvedVirtualCallParameterGenericsOffset;
    int32_t unresolvedVirtualCallParameterGenericsCount;
    int32_t resolvedVirtualCallParameterRuntimeTypesOffset;
    int32_t resolvedVirtualCallParameterRuntimeTypesCount;
    int32_t usablePermutationOffset;
    int32_t usablePermutationCount;
};

struct Il2CppTypeDefinition {
    uint32_t nameIndex;
    uint32_t namespaceIndex;
    uint32_t byvalTypeIndex;
    uint32_t byrefTypeIndex;
    uint32_t declaringTypeIndex;
    uint32_t parentIndex;
    uint32_t elementTypeIndex;
    uint32_t rgctxStartIndex;
    int32_t rgctxCount;
    uint32_t genericContainerIndex;
    uint32_t flags;
    uint32_t fieldStart;
    int32_t methodStart;
    uint32_t eventStart;
    uint32_t propertyStart;
    uint32_t nestedTypesStart;
    int32_t interfacesStart;
    int32_t vtableStart;
    int32_t interfaceOffsetsStart;
    uint16_t method_count;
    uint16_t property_count;
    uint16_t field_count;
    uint16_t event_count;
    uint16_t nested_type_count;
    uint16_t vtable_count;
    uint16_t interfaces_count;
    uint16_t interface_offsets_count;
    uint32_t bitfield;
    uint32_t token;
};

struct Il2CppMethodDefinition {
    uint32_t nameIndex;
    uint32_t declaringType;
    uint32_t returnType;
    uint32_t parameterStart;
    uint32_t genericContainerIndex;
    uint32_t methodIndex;
    uint32_t invokerIndex;
    uint32_t reversePInvokeWrapperIndex;
    uint32_t rgctxStartIndex;
    int32_t rgctxCount;
    uint32_t token;
    uint16_t flags;
    uint16_t iflags;
    uint16_t slot;
    uint16_t parameterCount;
};

struct Il2CppFieldDefinition {
    uint32_t nameIndex;
    uint32_t token;
    uint32_t typeIndex;
};

struct Il2CppImageDefinition {
    uint32_t nameIndex;
    uint32_t assemblyIndex;
    int32_t typeStart;
    uint32_t typeCount;
    int32_t exportedTypeStart;
    uint32_t exportedTypeCount;
    int32_t entryPointIndex;
    uint32_t token;
};

struct Il2CppAssemblyDefinition {
    uint32_t imageIndex;
    uint32_t tokenIndex;
    int32_t referencedAssemblyStart;
    int32_t referencedAssemblyCount;
    uint32_t anameIndex;
};

#pragma pack(pop)

// Runtime type info with inline caching support
struct RuntimeTypeInfo {
    uint32_t type_index;
    uint32_t parent_index;
    uint32_t element_type_index;
    uint32_t generic_inst_index;
    uint32_t flags;
    uint32_t instance_size;
    uint32_t native_size;
    uint32_t field_count;
    uint32_t method_count;
    uint32_t vtable_count;
    uint32_t interface_count;
    
    // Inline cache for fast type checks
    alignas(CACHE_LINE_SIZE) InlineCache type_check_cache;
    
    // Cached hash for fast equality checks
    uint64_t cached_hash;
    
    bool IsAbstract() const { return (flags & 0x0080) != 0; }
    bool IsInterface() const { return (flags & 0x0020) != 0; }
    bool IsSealed() const { return (flags & 0x0100) != 0; }
    bool IsValueType() const { return (flags & 0x10000) != 0; }
    bool IsEnum() const { return (flags & 0x4000) != 0; }
    bool IsClass() const { return !IsInterface() && !IsValueType(); }
};

// Method info with performance optimizations
struct RuntimeMethodInfo {
    uint32_t method_index;
    uint32_t declaring_type;
    uint32_t return_type;
    uint32_t parameter_count;
    uint32_t flags;
    uint32_t token;
    
    // Virtual dispatch cache
    uint32_t vtable_index;
    uint32_t slot;
    
    // Inlining hints
    bool is_inline_candidate;
    uint32_t code_size;
    
    // Native call bridge
    void* native_func;
    void* invoker_func;
    
    bool IsStatic() const { return (flags & 0x0010) != 0; }
    bool IsVirtual() const { return (flags & 0x0040) != 0; }
    bool IsAbstract() const { return (flags & 0x0400) != 0; }
    bool IsFinal() const { return (flags & 0x0020) != 0; }
    bool IsGeneric() const { return (flags & 0x00200000) != 0; }
};

// Metadata loader with full IL2CPP compatibility
class MetadataLoader {
public:
    MetadataLoader() = default;
    ~MetadataLoader() = default;
    
    bool LoadMetadata(const uint8_t* data, size_t size);
    bool LoadMetadataFromFile(const std::string& path);
    
    const Il2CppTypeDefinition* GetTypeDefinition(uint32_t index) const;
    const Il2CppMethodDefinition* GetMethodDefinition(uint32_t index) const;
    const Il2CppFieldDefinition* GetFieldDefinition(uint32_t index) const;
    const Il2CppImageDefinition* GetImageDefinition(uint32_t index) const;
    const Il2CppAssemblyDefinition* GetAssemblyDefinition(uint32_t index) const;
    
    const char* GetStringFromIndex(uint32_t index) const;
    
    uint32_t GetTypeDefinitionCount() const { return type_definitions_.size(); }
    uint32_t GetMethodDefinitionCount() const { return method_definitions_.size(); }
    
    // Create runtime type info from definition
    RuntimeTypeInfo CreateRuntimeType(uint32_t type_index);
    
    // Create runtime method info from definition
    RuntimeMethodInfo CreateRuntimeMethod(uint32_t method_index);
    
    // Resolve type from name
    uint32_t ResolveType(const char* name) const;
    uint32_t ResolveType(const char* name, const char* namespace_name) const;
    
    // Resolve method
    uint32_t ResolveMethod(uint32_t type_index, const char* method_name) const;
    
private:
    bool ParseMetadata(const uint8_t* data, size_t size);
    bool ValidateHeader(const Il2CppGlobalMetadataHeader* header);
    
    std::vector<Il2CppTypeDefinition> type_definitions_;
    std::vector<Il2CppMethodDefinition> method_definitions_;
    std::vector<Il2CppFieldDefinition> field_definitions_;
    std::vector<Il2CppImageDefinition> image_definitions_;
    std::vector<Il2CppAssemblyDefinition> assembly_definitions_;
    
    std::vector<uint8_t> string_table_;
    
    // Fast lookup caches
    std::unordered_map<uint64_t, uint32_t> type_name_cache_;
    std::unordered_map<uint64_t, uint32_t> method_name_cache_;
    
    bool loaded_ = false;
};

} // namespace hotc
