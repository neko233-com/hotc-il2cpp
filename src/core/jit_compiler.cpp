#include "jit_compiler.h"
#include <cstring>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__) || defined(__linux__)
#include <sys/mman.h>
#include <unistd.h>
#endif

namespace hotc {

// NativeCodeEmitter implementation

NativeCodeEmitter::NativeCodeEmitter(size_t initial_size) : size_(initial_size), offset_(0) {
    code_ = new uint8_t[size_];
}

NativeCodeEmitter::~NativeCodeEmitter() {
    delete[] code_;
}

void NativeCodeEmitter::Emit(uint8_t byte) {
    if (offset_ >= size_) Grow(1);
    code_[offset_++] = byte;
}

void NativeCodeEmitter::Emit32(uint32_t value) {
    if (offset_ + 4 > size_) Grow(4);
    std::memcpy(code_ + offset_, &value, 4);
    offset_ += 4;
}

void NativeCodeEmitter::Emit64(uint64_t value) {
    if (offset_ + 8 > size_) Grow(8);
    std::memcpy(code_ + offset_, &value, 8);
    offset_ += 8;
}

void NativeCodeEmitter::EmitREX(uint8_t rex) {
    if (rex != 0x40) Emit(rex);
}

void NativeCodeEmitter::EmitModRM(uint8_t mod, uint8_t reg, uint8_t rm) {
    Emit((mod << 6) | ((reg & 7) << 3) | (rm & 7));
}

void NativeCodeEmitter::EmitSIB(uint8_t scale, uint8_t index, uint8_t base) {
    Emit((scale << 6) | ((index & 7) << 3) | (base & 7));
}

void NativeCodeEmitter::MovRegImm32(uint8_t reg, uint32_t imm) {
    if (reg >= 8) EmitREX(0x41);
    Emit(0xB8 + (reg & 7));
    Emit32(imm);
}

void NativeCodeEmitter::MovRegImm64(uint8_t reg, uint64_t imm) {
    if (reg >= 8) EmitREX(0x49);
    Emit(0xB8 + (reg & 7));
    Emit64(imm);
}

void NativeCodeEmitter::MovMemReg(uint8_t base, int32_t offset, uint8_t reg) {
    if (base >= 8 || reg >= 8) EmitREX(0x48);
    Emit(0x89);
    EmitModRM(1, reg, base);
    Emit(static_cast<uint8_t>(offset));
}

void NativeCodeEmitter::MovRegMem(uint8_t reg, uint8_t base, int32_t offset) {
    if (base >= 8 || reg >= 8) EmitREX(0x48);
    Emit(0x8B);
    EmitModRM(1, reg, base);
    Emit(static_cast<uint8_t>(offset));
}

void NativeCodeEmitter::PushReg(uint8_t reg) {
    if (reg >= 8) EmitREX(0x41);
    Emit(0x50 + (reg & 7));
}

void NativeCodeEmitter::PopReg(uint8_t reg) {
    if (reg >= 8) EmitREX(0x41);
    Emit(0x58 + (reg & 7));
}

void NativeCodeEmitter::AddRegReg(uint8_t dst, uint8_t src) {
    if (dst >= 8 || src >= 8) EmitREX(0x49);
    Emit(0x01);
    EmitModRM(3, src, dst);
}

void NativeCodeEmitter::SubRegReg(uint8_t dst, uint8_t src) {
    if (dst >= 8 || src >= 8) EmitREX(0x49);
    Emit(0x29);
    EmitModRM(3, src, dst);
}

void NativeCodeEmitter::ImulRegReg(uint8_t dst, uint8_t src) {
    if (dst >= 8 || src >= 8) EmitREX(0x49);
    Emit(0x0F);
    Emit(0xAF);
    EmitModRM(3, dst, src);
}

void NativeCodeEmitter::XorRegReg(uint8_t dst, uint8_t src) {
    if (dst >= 8 || src >= 8) EmitREX(0x49);
    Emit(0x31);
    EmitModRM(3, src, dst);
}

void NativeCodeEmitter::CmpRegReg(uint8_t reg1, uint8_t reg2) {
    if (reg1 >= 8 || reg2 >= 8) EmitREX(0x49);
    Emit(0x39);
    EmitModRM(3, reg2, reg1);
}

void NativeCodeEmitter::Jcc(uint8_t cc, int32_t offset) {
    if (offset >= -128 && offset <= 127) {
        Emit(0x70 + cc);
        Emit(static_cast<uint8_t>(offset));
    } else {
        Emit(0x0F);
        Emit(0x80 + cc);
        Emit32(static_cast<uint32_t>(offset));
    }
}

void NativeCodeEmitter::Jmp(int32_t offset) {
    if (offset >= -128 && offset <= 127) {
        Emit(0xEB);
        Emit(static_cast<uint8_t>(offset));
    } else {
        Emit(0xE9);
        Emit32(static_cast<uint32_t>(offset));
    }
}

void NativeCodeEmitter::CallReg(uint8_t reg) {
    if (reg >= 8) EmitREX(0x41);
    Emit(0xFF);
    EmitModRM(3, 2, reg);
}

void NativeCodeEmitter::Ret() {
    Emit(0xC3);
}

void NativeCodeEmitter::Nop() {
    Emit(0x90);
}

void NativeCodeEmitter::Grow(size_t needed) {
    size_t new_size = size_ * 2;
    while (new_size < offset_ + needed) new_size *= 2;
    
    uint8_t* new_code = new uint8_t[new_size];
    std::memcpy(new_code, code_, offset_);
    delete[] code_;
    code_ = new_code;
    size_ = new_size;
}

// JITCompiler implementation

CompiledCode* JITCompiler::Compile(const MethodBody& method, uint32_t method_index) {
    emitter_.Reset();
    
    // Prologue: push rbp, mov rbp, rsp
    emitter_.PushReg(REG_RBP);
    emitter_.MovRegImm64(REG_RBP, 0);
    // mov rbp, rsp
    emitter_.Emit(0x48);
    emitter_.Emit(0x89);
    emitter_.EmitModRM(3, REG_RSP, REG_RBP);
    
    // Allocate space for locals
    uint32_t locals_size = method.max_locals * 8;
    if (locals_size > 0) {
        // sub rsp, imm32
        emitter_.Emit(0x48);
        emitter_.Emit(0x81);
        emitter_.EmitModRM(3, 5, REG_RSP);
        emitter_.Emit32(locals_size);
    }
    
    // Compile each instruction
    for (const auto& instr : method.instructions) {
        switch (instr.opcode) {
            case OpCode::Ldc_I4_0:
            case OpCode::Ldc_I4_1:
            case OpCode::Ldc_I4_2:
            case OpCode::Ldc_I4_3:
            case OpCode::Ldc_I4_4:
            case OpCode::Ldc_I4_5:
            case OpCode::Ldc_I4_6:
            case OpCode::Ldc_I4_7:
            case OpCode::Ldc_I4_8: {
                int32_t imm = static_cast<int32_t>(instr.opcode) - 0x17;
                emitter_.MovRegImm32(REG_RAX, imm);
                break;
            }
            
            case OpCode::Ldc_I4:
            case OpCode::Ldc_I4_S:
                emitter_.MovRegImm32(REG_RAX, instr.operand);
                break;
            
            case OpCode::Add:
                // pop rbx, add rax, rbx
                emitter_.PopReg(REG_RBX);
                emitter_.AddRegReg(REG_RAX, REG_RBX);
                break;
            
            case OpCode::Sub:
                emitter_.PopReg(REG_RBX);
                emitter_.SubRegReg(REG_RAX, REG_RBX);
                break;
            
            case OpCode::Mul:
                emitter_.PopReg(REG_RBX);
                emitter_.ImulRegReg(REG_RAX, REG_RBX);
                break;
            
            case OpCode::Ret:
                break;
            
            default:
                break;
        }
    }
    
    // Epilogue: mov rsp, rbp; pop rbp; ret
    emitter_.MovRegMem(REG_RSP, REG_RBP, 0);
    emitter_.PopReg(REG_RBP);
    emitter_.Ret();
    
    // Create compiled code object
    auto compiled = std::make_unique<CompiledCode>();
    compiled->size = emitter_.GetSize();
    compiled->code = new uint8_t[compiled->size];
    std::memcpy(compiled->code, emitter_.GetCode(), compiled->size);
    compiled->method_index = method_index;
    compiled->is_valid = true;
    
    // Make code executable
#if defined(_WIN32)
    DWORD old_protect;
    VirtualProtect(compiled->code, compiled->size, PAGE_EXECUTE_READWRITE, &old_protect);
#elif defined(__APPLE__) || defined(__linux__)
    long page_size = sysconf(_SC_PAGESIZE);
    uintptr_t addr = reinterpret_cast<uintptr_t>(compiled->code);
    uintptr_t page_start = addr & ~(page_size - 1);
    mprotect(reinterpret_cast<void*>(page_start), 
             compiled->size + (addr - page_start),
             PROT_READ | PROT_WRITE | PROT_EXEC);
#endif
    
    CompiledCode* result = compiled.get();
    compiled_methods_[method_index] = std::move(compiled);
    
    return result;
}

bool JITCompiler::ShouldCompile(uint32_t method_index, uint64_t execution_count) const {
    return execution_count >= HOT_THRESHOLD;
}

CompiledCode* JITCompiler::GetCompiledCode(uint32_t method_index) const {
    auto it = compiled_methods_.find(method_index);
    return (it != compiled_methods_.end() && it->second->is_valid) ? it->second.get() : nullptr;
}

void JITCompiler::InvalidateMethod(uint32_t method_index) {
    compiled_methods_.erase(method_index);
}

void JITCompiler::InvalidateAll() {
    compiled_methods_.clear();
}

uint64_t JITCompiler::GetTotalCompiledBytes() const {
    uint64_t total = 0;
    for (const auto& [idx, code] : compiled_methods_) {
        total += code->size;
    }
    return total;
}

double JITCompiler::GetHitRate() const {
    uint64_t compiled = 0;
    uint64_t total = 0;
    for (const auto& [idx, count] : execution_counts_) {
        total += count;
        if (compiled_methods_.count(idx)) compiled += count;
    }
    return total > 0 ? static_cast<double>(compiled) / total : 0.0;
}

void JITCompiler::EmitArithmeticOp(NativeCodeEmitter& emitter, uint8_t opcode) {
    emitter.PopReg(REG_RBX);
    emitter.Emit(0x48);
    emitter.Emit(opcode);
    emitter.EmitModRM(3, REG_RBX, REG_RAX);
}

void JITCompiler::EmitComparisonOp(NativeCodeEmitter& emitter, uint8_t setcc) {
    emitter.PopReg(REG_RBX);
    emitter.CmpRegReg(REG_RAX, REG_RBX);
    emitter.Emit(0x0F);
    emitter.Emit(setcc);
    emitter.EmitModRM(3, 0, REG_RAX);
    emitter.Emit(0x48);
    emitter.Emit(0x0F);
    emitter.Emit(0xB6);
    emitter.EmitModRM(3, REG_RAX, REG_RAX);
}

void JITCompiler::EmitBranchOp(NativeCodeEmitter& emitter, uint8_t cc, int32_t target) {
    // Placeholder - would need relocation
    emitter.Jcc(cc, target);
}

void JITCompiler::EmitMethodCall(NativeCodeEmitter& emitter, uint32_t target_method) {
    // Placeholder for method dispatch
    emitter.Nop();
}

void JITCompiler::EmitReturn(NativeCodeEmitter& emitter) {
    emitter.MovRegMem(REG_RSP, REG_RBP, 0);
    emitter.PopReg(REG_RBP);
    emitter.Ret();
}

// InlineCacheManager implementation

void* InlineCacheManager::GetCachedTarget(uint32_t type_id, uint32_t slot, CacheEntry* entries, uint32_t count) {
    for (uint32_t i = 0; i < count && i < MAXPIC_ENTRIES; i++) {
        if (entries[i].type_id == type_id && entries[i].version == 1) {
            return entries[i].target;
        }
    }
    return nullptr;
}

void InlineCacheManager::UpdateCache(uint32_t type_id, uint32_t slot, void* target, CacheEntry* entries, uint32_t count) {
    // Find empty slot or replace oldest
    for (uint32_t i = 0; i < MAXPIC_ENTRIES; i++) {
        if (entries[i].version == 0 || i >= count) {
            entries[i].type_id = type_id;
            entries[i].version = 1;
            entries[i].target = target;
            return;
        }
    }
    // Replace first entry if full
    entries[0].type_id = type_id;
    entries[0].version = 1;
    entries[0].target = target;
}

uint64_t InlineCacheManager::GetCacheKey(uint32_t type_id, uint32_t slot) const {
    return (static_cast<uint64_t>(type_id) << 32) | slot;
}

} // namespace hotc
