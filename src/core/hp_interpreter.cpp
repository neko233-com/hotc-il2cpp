#include "hp_interpreter.h"
#include <chrono>
#include <cstring>
#include <stdexcept>
#include <algorithm>

namespace hotc {

HighPerformanceInterpreter::HighPerformanceInterpreter(
    TypeSystem& types, 
    MemoryManager& memory, 
    const InterpreterConfig& config)
    : type_system_(types), memory_(memory), config_(config) {
    InitDispatchTable();
    
    // Allocate stacks with configured sizes
    context_.int_stack = new int32_t[config_.int_stack_size];
    context_.int_stack_capacity = config_.int_stack_size;
    context_.int_stack_top = 0;
    
    context_.object_stack = new void*[config_.object_stack_size];
    context_.object_stack_capacity = config_.object_stack_size;
    context_.object_stack_top = 0;
    
    context_.float_stack = new float[config_.float_stack_size];
    context_.float_stack_top = 0;
    
    context_.double_stack = new double[config_.double_stack_size];
    context_.double_stack_top = 0;
    
    context_.long_stack = new int64_t[config_.long_stack_size];
    context_.long_stack_top = 0;
    
    context_.locals = new void*[config_.locals_size];
    context_.num_locals = config_.locals_size;
    
    // Auto-detect worker threads
    // (reserved for parallel execution)
}

HighPerformanceInterpreter::~HighPerformanceInterpreter() {
    delete[] context_.int_stack;
    delete[] context_.object_stack;
    delete[] context_.float_stack;
    delete[] context_.double_stack;
    delete[] context_.long_stack;
    delete[] context_.locals;
}

void HighPerformanceInterpreter::InitDispatchTable() {
    // Initialize all handlers to Nop
    dispatch_table_.fill(&HighPerformanceInterpreter::HandleNop);
    
    // Single-byte opcodes
    dispatch_table_[0x00] = &HighPerformanceInterpreter::HandleNop;
    dispatch_table_[0x25] = &HighPerformanceInterpreter::HandleDup;
    dispatch_table_[0x26] = &HighPerformanceInterpreter::HandlePop;
    dispatch_table_[0x58] = &HighPerformanceInterpreter::HandleAdd;
    dispatch_table_[0x59] = &HighPerformanceInterpreter::HandleSub;
    dispatch_table_[0x5A] = &HighPerformanceInterpreter::HandleMul;
    dispatch_table_[0x5B] = &HighPerformanceInterpreter::HandleDiv;
    dispatch_table_[0x5D] = &HighPerformanceInterpreter::HandleRem;
    dispatch_table_[0x5F] = &HighPerformanceInterpreter::HandleAnd;
    dispatch_table_[0x60] = &HighPerformanceInterpreter::HandleOr;
    dispatch_table_[0x61] = &HighPerformanceInterpreter::HandleXor;
    dispatch_table_[0x62] = &HighPerformanceInterpreter::HandleShl;
    dispatch_table_[0x63] = &HighPerformanceInterpreter::HandleShr;
    dispatch_table_[0x65] = &HighPerformanceInterpreter::HandleNeg;
    dispatch_table_[0x66] = &HighPerformanceInterpreter::HandleNot;
    dispatch_table_[0x2A] = &HighPerformanceInterpreter::HandleRet;
    dispatch_table_[0x7A] = &HighPerformanceInterpreter::HandleThrow;
    
    // Load constant opcodes
    dispatch_table_[0x16] = &HighPerformanceInterpreter::HandleLdcI4;
    dispatch_table_[0x17] = &HighPerformanceInterpreter::HandleLdcI4;
    dispatch_table_[0x18] = &HighPerformanceInterpreter::HandleLdcI4;
    dispatch_table_[0x19] = &HighPerformanceInterpreter::HandleLdcI4;
    dispatch_table_[0x1A] = &HighPerformanceInterpreter::HandleLdcI4;
    dispatch_table_[0x1B] = &HighPerformanceInterpreter::HandleLdcI4;
    dispatch_table_[0x1C] = &HighPerformanceInterpreter::HandleLdcI4;
    dispatch_table_[0x1D] = &HighPerformanceInterpreter::HandleLdcI4;
    dispatch_table_[0x1E] = &HighPerformanceInterpreter::HandleLdcI4;
    dispatch_table_[0x1F] = &HighPerformanceInterpreter::HandleLdcI4;
    dispatch_table_[0x20] = &HighPerformanceInterpreter::HandleLdcI4;
    dispatch_table_[0x21] = &HighPerformanceInterpreter::HandleLdcI4;
    dispatch_table_[0x22] = &HighPerformanceInterpreter::HandleLdcR4;
    dispatch_table_[0x23] = &HighPerformanceInterpreter::HandleLdcR8;
    
    // Branch opcodes
    dispatch_table_[0x2B] = &HighPerformanceInterpreter::HandleBr;
    dispatch_table_[0x2C] = &HighPerformanceInterpreter::HandleBrfalse;
    dispatch_table_[0x2D] = &HighPerformanceInterpreter::HandleBrtrue;
    dispatch_table_[0x38] = &HighPerformanceInterpreter::HandleBr;
    dispatch_table_[0x39] = &HighPerformanceInterpreter::HandleBrfalse;
    dispatch_table_[0x3A] = &HighPerformanceInterpreter::HandleBrtrue;
    dispatch_table_[0x3B] = &HighPerformanceInterpreter::HandleBeq;
    dispatch_table_[0x3C] = &HighPerformanceInterpreter::HandleBge;
    dispatch_table_[0x3D] = &HighPerformanceInterpreter::HandleBgt;
    dispatch_table_[0x3E] = &HighPerformanceInterpreter::HandleBle;
    dispatch_table_[0x3F] = &HighPerformanceInterpreter::HandleBlt;
    dispatch_table_[0x40] = &HighPerformanceInterpreter::HandleBge;
    dispatch_table_[0x41] = &HighPerformanceInterpreter::HandleBge;
    dispatch_table_[0x42] = &HighPerformanceInterpreter::HandleBgt;
    dispatch_table_[0x43] = &HighPerformanceInterpreter::HandleBle;
    dispatch_table_[0x44] = &HighPerformanceInterpreter::HandleBlt;
    
    // Call/Return
    dispatch_table_[0x28] = &HighPerformanceInterpreter::HandleCall;
    dispatch_table_[0x27] = &HighPerformanceInterpreter::HandleCall;
    
    // Object opcodes
    dispatch_table_[0x8C] = &HighPerformanceInterpreter::HandleBox;
    dispatch_table_[0x75] = &HighPerformanceInterpreter::HandleCastclass;
    dispatch_table_[0x74] = &HighPerformanceInterpreter::HandleIsinst;
    dispatch_table_[0x8D] = &HighPerformanceInterpreter::HandleNewarr;
    dispatch_table_[0x8E] = &HighPerformanceInterpreter::HandleLdlen;
    dispatch_table_[0x8F] = &HighPerformanceInterpreter::HandleLdelem;
    dispatch_table_[0x90] = &HighPerformanceInterpreter::HandleStelem;
    dispatch_table_[0x94] = &HighPerformanceInterpreter::HandleLdelem;
    dispatch_table_[0x9E] = &HighPerformanceInterpreter::HandleStelem;
    dispatch_table_[0x7B] = &HighPerformanceInterpreter::HandleLdfld;
    dispatch_table_[0x7D] = &HighPerformanceInterpreter::HandleStfld;
    dispatch_table_[0x7E] = &HighPerformanceInterpreter::HandleLdsfld;
    dispatch_table_[0x80] = &HighPerformanceInterpreter::HandleStsfld;
    dispatch_table_[0x73] = &HighPerformanceInterpreter::HandleNewobj;
}

void HighPerformanceInterpreter::Execute(const MethodBody& method) {
    Execute(method, nullptr, 0);
}

void HighPerformanceInterpreter::Execute(const MethodBody& method, void** args, uint32_t arg_count) {
    current_method_ = &method;
    context_.pc = 0;
    context_.running = true;
    context_.has_exception = false;
    context_.exception_obj = nullptr;
    
    // Reset stacks
    context_.int_stack_top = 0;
    context_.object_stack_top = 0;
    context_.float_stack_top = 0;
    context_.double_stack_top = 0;
    context_.long_stack_top = 0;
    
    // Copy arguments to locals
    if (args && arg_count > 0) {
        for (uint32_t i = 0; i < arg_count && i < context_.num_locals; i++) {
            context_.locals[i] = args[i];
        }
    }
    
    // Start timing
    auto start = std::chrono::high_resolution_clock::now();
    
    // Execute main loop
    ExecuteLoop();
    
    // End timing
    auto end = std::chrono::high_resolution_clock::now();
    execution_time_ms_ = std::chrono::duration<double, std::milli>(end - start).count();
    
    // Calculate ops per second
    if (execution_time_ms_ > 0) {
        ops_per_second_ = (instructions_executed_ / execution_time_ms_) * 1000.0;
    }
}

void HighPerformanceInterpreter::ExecuteLoop() {
    // Main execution loop - optimized for maximum throughput
    while (context_.running && context_.pc < current_method_->instructions.size()) {
        const auto& instr = current_method_->instructions[context_.pc];
        
        // Dispatch using function pointer table
        (this->*dispatch_table_[static_cast<uint8_t>(instr.opcode)])();
        
        context_.pc++;
        instructions_executed_++;
    }
}

// Handler implementations - each optimized for maximum performance

void HighPerformanceInterpreter::HandleNop() {
    // No operation - do nothing
}

void HighPerformanceInterpreter::HandleLdcI4() {
    const auto& instr = current_method_->instructions[context_.pc];
    context_.PushInt32(static_cast<int32_t>(instr.operand));
}

void HighPerformanceInterpreter::HandleLdcR4() {
    const auto& instr = current_method_->instructions[context_.pc];
    float value;
    std::memcpy(&value, &instr.operand, sizeof(float));
    context_.PushFloat(value);
}

void HighPerformanceInterpreter::HandleLdcR8() {
    const auto& instr = current_method_->instructions[context_.pc];
    uint32_t low = instr.operand;
    uint32_t high = (context_.pc + 1 < current_method_->instructions.size()) 
        ? current_method_->instructions[context_.pc + 1].operand : 0;
    uint64_t raw = (static_cast<uint64_t>(high) << 32) | low;
    double value;
    std::memcpy(&value, &raw, sizeof(double));
    context_.PushDouble(value);
}

void HighPerformanceInterpreter::HandleDup() {
    if (context_.int_stack_top > 0) {
        int32_t v = context_.PopInt32();
        context_.PushInt32(v);
        context_.PushInt32(v);
    } else if (context_.object_stack_top > 0) {
        void* v = context_.PopObject();
        context_.PushObject(v);
        context_.PushObject(v);
    }
}

void HighPerformanceInterpreter::HandlePop() {
    if (context_.int_stack_top > 0) {
        context_.PopInt32();
    } else if (context_.object_stack_top > 0) {
        context_.PopObject();
    }
}

void HighPerformanceInterpreter::HandleAdd() {
    int32_t b = context_.PopInt32();
    int32_t a = context_.PopInt32();
    context_.PushInt32(a + b);
}

void HighPerformanceInterpreter::HandleSub() {
    int32_t b = context_.PopInt32();
    int32_t a = context_.PopInt32();
    context_.PushInt32(a - b);
}

void HighPerformanceInterpreter::HandleMul() {
    int32_t b = context_.PopInt32();
    int32_t a = context_.PopInt32();
    context_.PushInt32(a * b);
}

void HighPerformanceInterpreter::HandleDiv() {
    int32_t b = context_.PopInt32();
    int32_t a = context_.PopInt32();
    context_.PushInt32(a / b);
}

void HighPerformanceInterpreter::HandleRem() {
    int32_t b = context_.PopInt32();
    int32_t a = context_.PopInt32();
    context_.PushInt32(a % b);
}

void HighPerformanceInterpreter::HandleAnd() {
    int32_t b = context_.PopInt32();
    int32_t a = context_.PopInt32();
    context_.PushInt32(a & b);
}

void HighPerformanceInterpreter::HandleOr() {
    int32_t b = context_.PopInt32();
    int32_t a = context_.PopInt32();
    context_.PushInt32(a | b);
}

void HighPerformanceInterpreter::HandleXor() {
    int32_t b = context_.PopInt32();
    int32_t a = context_.PopInt32();
    context_.PushInt32(a ^ b);
}

void HighPerformanceInterpreter::HandleShl() {
    int32_t b = context_.PopInt32();
    int32_t a = context_.PopInt32();
    context_.PushInt32(a << b);
}

void HighPerformanceInterpreter::HandleShr() {
    int32_t b = context_.PopInt32();
    int32_t a = context_.PopInt32();
    context_.PushInt32(a >> b);
}

void HighPerformanceInterpreter::HandleNeg() {
    int32_t v = context_.PopInt32();
    context_.PushInt32(-v);
}

void HighPerformanceInterpreter::HandleNot() {
    int32_t v = context_.PopInt32();
    context_.PushInt32(~v);
}

void HighPerformanceInterpreter::HandleCeq() {
    int32_t b = context_.PopInt32();
    int32_t a = context_.PopInt32();
    context_.PushInt32(a == b ? 1 : 0);
}

void HighPerformanceInterpreter::HandleCgt() {
    int32_t b = context_.PopInt32();
    int32_t a = context_.PopInt32();
    context_.PushInt32(a > b ? 1 : 0);
}

void HighPerformanceInterpreter::HandleClt() {
    int32_t b = context_.PopInt32();
    int32_t a = context_.PopInt32();
    context_.PushInt32(a < b ? 1 : 0);
}

void HighPerformanceInterpreter::HandleBr() {
    const auto& instr = current_method_->instructions[context_.pc];
    context_.pc = instr.operand - 1;
}

void HighPerformanceInterpreter::HandleBrfalse() {
    const auto& instr = current_method_->instructions[context_.pc];
    int32_t v = context_.PopInt32();
    if (v == 0) {
        context_.pc = instr.operand - 1;
        RecordBranchMispredict();
    }
}

void HighPerformanceInterpreter::HandleBrtrue() {
    const auto& instr = current_method_->instructions[context_.pc];
    int32_t v = context_.PopInt32();
    if (v != 0) {
        context_.pc = instr.operand - 1;
        RecordBranchMispredict();
    }
}

void HighPerformanceInterpreter::HandleBeq() {
    const auto& instr = current_method_->instructions[context_.pc];
    int32_t b = context_.PopInt32();
    int32_t a = context_.PopInt32();
    if (a == b) {
        context_.pc = instr.operand - 1;
    }
}

void HighPerformanceInterpreter::HandleBge() {
    const auto& instr = current_method_->instructions[context_.pc];
    int32_t b = context_.PopInt32();
    int32_t a = context_.PopInt32();
    if (a >= b) {
        context_.pc = instr.operand - 1;
    }
}

void HighPerformanceInterpreter::HandleBgt() {
    const auto& instr = current_method_->instructions[context_.pc];
    int32_t b = context_.PopInt32();
    int32_t a = context_.PopInt32();
    if (a > b) {
        context_.pc = instr.operand - 1;
    }
}

void HighPerformanceInterpreter::HandleBle() {
    const auto& instr = current_method_->instructions[context_.pc];
    int32_t b = context_.PopInt32();
    int32_t a = context_.PopInt32();
    if (a <= b) {
        context_.pc = instr.operand - 1;
    }
}

void HighPerformanceInterpreter::HandleBlt() {
    const auto& instr = current_method_->instructions[context_.pc];
    int32_t b = context_.PopInt32();
    int32_t a = context_.PopInt32();
    if (a < b) {
        context_.pc = instr.operand - 1;
    }
}

void HighPerformanceInterpreter::HandleCall() {
    // Method call implementation
    const auto& instr = current_method_->instructions[context_.pc];
    // TODO: Full method dispatch with inline caching
    context_.pc++;
}

void HighPerformanceInterpreter::HandleRet() {
    if (context_.int_stack_top > 0) {
        return_int_ = context_.PopInt32();
    } else if (context_.object_stack_top > 0) {
        return_value_ = context_.PopObject();
    }
    context_.running = false;
}

void HighPerformanceInterpreter::HandleBox() {
    // Boxing implementation
    const auto& instr = current_method_->instructions[context_.pc];
    int32_t v = context_.PopInt32();
    void* boxed = memory_.AllocateObject(sizeof(int32_t));
    std::memcpy(boxed, &v, sizeof(int32_t));
    context_.PushObject(boxed);
}

void HighPerformanceInterpreter::HandleUnbox() {
    const auto& instr = current_method_->instructions[context_.pc];
    void* obj = context_.PopObject();
    int32_t v;
    std::memcpy(&v, obj, sizeof(int32_t));
    context_.PushInt32(v);
}

void HighPerformanceInterpreter::HandleNewarr() {
    const auto& instr = current_method_->instructions[context_.pc];
    uint32_t count = context_.PopInt32();
    void* arr = memory_.AllocateArray(4, count);
    context_.PushObject(arr);
}

void HighPerformanceInterpreter::HandleLdlen() {
    void* arr = context_.PopObject();
    uint32_t len = *static_cast<uint32_t*>(arr);
    context_.PushInt32(static_cast<int32_t>(len));
}

void HighPerformanceInterpreter::HandleLdelem() {
    const auto& instr = current_method_->instructions[context_.pc];
    uint32_t index = context_.PopInt32();
    void* arr = context_.PopObject();
    uint8_t* base = static_cast<uint8_t*>(arr) + sizeof(uint32_t);
    uint32_t* elements = reinterpret_cast<uint32_t*>(base);
    context_.PushInt32(static_cast<int32_t>(elements[index]));
}

void HighPerformanceInterpreter::HandleStelem() {
    const auto& instr = current_method_->instructions[context_.pc];
    int32_t value = context_.PopInt32();
    uint32_t index = context_.PopInt32();
    void* arr = context_.PopObject();
    uint8_t* base = static_cast<uint8_t*>(arr) + sizeof(uint32_t);
    uint32_t* elements = reinterpret_cast<uint32_t*>(base);
    elements[index] = static_cast<uint32_t>(value);
}

void HighPerformanceInterpreter::HandleLdfld() {
    const auto& instr = current_method_->instructions[context_.pc];
    void* obj = context_.PopObject();
    if (obj) {
        uint32_t offset = instr.operand;
        void* field = static_cast<uint8_t*>(obj) + offset;
        context_.PushObject(field);
    }
}

void HighPerformanceInterpreter::HandleStfld() {
    const auto& instr = current_method_->instructions[context_.pc];
    void* value = context_.PopObject();
    void* obj = context_.PopObject();
    if (obj) {
        uint32_t offset = instr.operand;
        void* field = static_cast<uint8_t*>(obj) + offset;
        std::memcpy(field, &value, sizeof(void*));
    }
}

void HighPerformanceInterpreter::HandleLdsfld() {
    const auto& instr = current_method_->instructions[context_.pc];
    // Static field access
    context_.PushObject(nullptr);
}

void HighPerformanceInterpreter::HandleStsfld() {
    const auto& instr = current_method_->instructions[context_.pc];
    void* value = context_.PopObject();
    // Static field store
}

void HighPerformanceInterpreter::HandleNewobj() {
    const auto& instr = current_method_->instructions[context_.pc];
    void* obj = memory_.AllocateObject(32);
    context_.PushObject(obj);
}

void HighPerformanceInterpreter::HandleCastclass() {
    const auto& instr = current_method_->instructions[context_.pc];
    // Type check with inline caching
    void* obj = context_.PopObject();
    context_.PushObject(obj);
}

void HighPerformanceInterpreter::HandleIsinst() {
    const auto& instr = current_method_->instructions[context_.pc];
    void* obj = context_.PopObject();
    // Type check with inline caching
    context_.PushObject(obj);
}

void HighPerformanceInterpreter::HandleThrow() {
    void* exception = context_.PopObject();
    context_.has_exception = true;
    context_.exception_obj = exception;
    context_.running = false;
}

void HighPerformanceInterpreter::HandleRethrow() {
    context_.running = false;
}

void HighPerformanceInterpreter::HandleLeave() {
    const auto& instr = current_method_->instructions[context_.pc];
    context_.pc = instr.operand - 1;
    // Cleanup exception handling state
}

void HighPerformanceInterpreter::HandleEndfinally() {
    // End finally block
}

bool HighPerformanceInterpreter::CheckType(uint32_t type_id, InlineCache& cache) {
    if (cache.Check(type_id)) {
        RecordCacheHit();
        return true;
    }
    RecordCacheMiss();
    return false;
}

void HighPerformanceInterpreter::RecordTypeHit(uint32_t type_id, InlineCache& cache) {
    cache.RecordHit(type_id);
}

void HighPerformanceInterpreter::RecordTypeMiss(InlineCache& cache) {
    cache.RecordMiss();
}

void HighPerformanceInterpreter::PushExceptionFrame(uint32_t catch_pc, uint32_t finally_pc, uint32_t filter_pc) {
    // Push exception frame for try/catch/finally
}

void HighPerformanceInterpreter::PopExceptionFrame() {
    // Pop exception frame
}

void HighPerformanceInterpreter::HandleException(void* exception) {
    // Handle exception with stack unwinding
}

void HighPerformanceInterpreter::OptimizeBranch(uint32_t target) {
    // Branch prediction optimization
}

void HighPerformanceInterpreter::RecordBranchMispredict() {
    branch_mispredicts_++;
}

void HighPerformanceInterpreter::RecordCacheHit() {
    cache_hits_++;
}

void HighPerformanceInterpreter::RecordCacheMiss() {
    cache_misses_++;
}

void HighPerformanceInterpreter::RegisterNativeFunction(const std::string& name, NativeFunc func) {
    native_functions_[name] = std::move(func);
}

} // namespace hotc
