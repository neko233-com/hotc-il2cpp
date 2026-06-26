#pragma once

#include "platform.h"
#include "runtime.h"
#include "hp_interpreter.h"
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <memory>
#include <cstring>

namespace hotc {

// JIT compiled code buffer
struct CompiledCode {
    uint8_t* code;
    size_t size;
    uint32_t method_index;
    uint64_t execution_count;
    bool is_valid;
    
    CompiledCode() : code(nullptr), size(0), method_index(0), execution_count(0), is_valid(false) {}
    ~CompiledCode() { if (code) delete[] code; }
    
    CompiledCode(const CompiledCode&) = delete;
    CompiledCode& operator=(const CompiledCode&) = delete;
    
    CompiledCode(CompiledCode&& other) noexcept 
        : code(other.code), size(other.size), method_index(other.method_index),
          execution_count(other.execution_count), is_valid(other.is_valid) {
        other.code = nullptr;
        other.is_valid = false;
    }
};

// x86-64 native code emitter
class NativeCodeEmitter {
public:
    NativeCodeEmitter(size_t initial_size = 4096);
    ~NativeCodeEmitter();
    
    // Emit x86-64 instructions
    void Emit(uint8_t byte);
    void Emit32(uint32_t value);
    void Emit64(uint64_t value);
    void EmitREX(uint8_t rex);
    void EmitModRM(uint8_t mod, uint8_t reg, uint8_t rm);
    void EmitSIB(uint8_t scale, uint8_t index, uint8_t base);
    
    // Register operations
    void MovRegImm32(uint8_t reg, uint32_t imm);
    void MovRegImm64(uint8_t reg, uint64_t imm);
    void MovMemReg(uint8_t base, int32_t offset, uint8_t reg);
    void MovRegMem(uint8_t reg, uint8_t base, int32_t offset);
    void PushReg(uint8_t reg);
    void PopReg(uint8_t reg);
    void AddRegReg(uint8_t dst, uint8_t src);
    void SubRegReg(uint8_t dst, uint8_t src);
    void ImulRegReg(uint8_t dst, uint8_t src);
    void XorRegReg(uint8_t dst, uint8_t src);
    void CmpRegReg(uint8_t reg1, uint8_t reg2);
    void Jcc(uint8_t cc, int32_t offset);
    void Jmp(int32_t offset);
    void CallReg(uint8_t reg);
    void Ret();
    void Nop();
    
    // Get compiled code
    uint8_t* GetCode() const { return code_; }
    size_t GetSize() const { return offset_; }
    
    // Reset for reuse
    void Reset() { offset_ = 0; }
    
private:
    void Grow(size_t needed);
    
    uint8_t* code_;
    size_t size_;
    size_t offset_;
};

// JIT compiler for hot methods
class JITCompiler {
public:
    JITCompiler() = default;
    ~JITCompiler() = default;
    
    // Compile a method to native code
    CompiledCode* Compile(const MethodBody& method, uint32_t method_index);
    
    // Check if method should be compiled (hot threshold)
    bool ShouldCompile(uint32_t method_index, uint64_t execution_count) const;
    
    // Get compiled code for method
    CompiledCode* GetCompiledCode(uint32_t method_index) const;
    
    // Invalidation
    void InvalidateMethod(uint32_t method_index);
    void InvalidateAll();
    
    // Statistics
    uint32_t GetCompiledMethodCount() const { return compiled_methods_.size(); }
    uint64_t GetTotalCompiledBytes() const;
    double GetHitRate() const;
    
private:
    // Simple x86-64 code generation
    void EmitArithmeticOp(NativeCodeEmitter& emitter, uint8_t opcode);
    void EmitComparisonOp(NativeCodeEmitter& emitter, uint8_t setcc);
    void EmitBranchOp(NativeCodeEmitter& emitter, uint8_t cc, int32_t target);
    void EmitMethodCall(NativeCodeEmitter& emitter, uint32_t target_method);
    void EmitReturn(NativeCodeEmitter& emitter);
    
    // Register allocation (simple linear scan)
    static constexpr uint8_t REG_RAX = 0;
    static constexpr uint8_t REG_RCX = 1;
    static constexpr uint8_t REG_RDX = 2;
    static constexpr uint8_t REG_RBX = 3;
    static constexpr uint8_t REG_RSP = 4;
    static constexpr uint8_t REG_RBP = 5;
    static constexpr uint8_t REG_RSI = 6;
    static constexpr uint8_t REG_RDI = 7;
    static constexpr uint8_t REG_R8 = 8;
    static constexpr uint8_t REG_R9 = 9;
    static constexpr uint8_t REG_R10 = 10;
    static constexpr uint8_t REG_R11 = 11;
    
    // Hot threshold for compilation
    static constexpr uint64_t HOT_THRESHOLD = 10000;
    
    std::unordered_map<uint32_t, std::unique_ptr<CompiledCode>> compiled_methods_;
    std::unordered_map<uint32_t, uint64_t> execution_counts_;
    NativeCodeEmitter emitter_;
};

// Inline cache for virtual calls
class InlineCacheManager {
public:
    InlineCacheManager() = default;
    ~InlineCacheManager() = default;
    
    struct CacheEntry {
        uint32_t type_id;
        uint32_t version;
        void* target;
    };
    
    // Get cached virtual call target
    void* GetCachedTarget(uint32_t type_id, uint32_t slot, CacheEntry* entries, uint32_t count);
    
    // Update cache
    void UpdateCache(uint32_t type_id, uint32_t slot, void* target, CacheEntry* entries, uint32_t count);
    
    // Polymorphic inline cache (up to 4 entries)
    static constexpr uint32_t MAXPIC_ENTRIES = 4;
    
private:
    uint64_t GetCacheKey(uint32_t type_id, uint32_t slot) const;
};

} // namespace hotc
