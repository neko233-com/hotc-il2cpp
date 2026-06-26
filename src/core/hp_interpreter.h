#pragma once

#include "platform.h"
#include "runtime.h"
#include "metadata.h"
#include "bridge.h"
#include "il_parser.h"
#include "type_system.h"
#include "memory.h"
#include <vector>
#include <functional>
#include <cstring>
#include <array>
#include <atomic>

namespace hotc {

// Forward declarations
class Interpreter;

// Native function signature
using NativeFunc = std::function<void* (void** args, uint32_t arg_count)>;

// Exception handling frame
struct ExceptionFrame {
    uint32_t catch_pc;
    uint32_t finally_pc;
    uint32_t filter_pc;
    uint32_t stack_depth;
    void* exception;
    ExceptionFrame* prev;
};

// Interpreter configuration for maximum performance
struct InterpreterConfig {
    // Stack sizes (tuned for performance)
    uint32_t int_stack_size = 2048;
    uint32_t object_stack_size = 512;
    uint32_t float_stack_size = 512;
    uint32_t double_stack_size = 256;
    uint32_t long_stack_size = 256;
    uint32_t locals_size = 256;
    
    // Performance options
    bool enable_inline_cache = true;
    bool enable_branch_prediction = true;
    bool enable_constant_folding = true;
    bool enable_dead_code_elimination = true;
    bool enable_tail_call_optimization = true;
    bool enable_exception_handlers = true;
    
    // Threading options
    uint32_t worker_threads = 0; // 0 = auto-detect
    bool enable_parallel_execution = false;
};

// High-performance interpreter with all optimizations
class HighPerformanceInterpreter {
public:
    HighPerformanceInterpreter(TypeSystem& types, MemoryManager& memory, const InterpreterConfig& config = {});
    ~HighPerformanceInterpreter();
    
    // Execute method with maximum performance
    void Execute(const MethodBody& method);
    void Execute(const MethodBody& method, void** args, uint32_t arg_count);
    
    // Register native function
    void RegisterNativeFunction(const std::string& name, NativeFunc func);
    
    // Get return value
    void* GetReturnValue() const { return return_value_; }
    int32_t GetReturnInt() const { return return_int_; }
    float GetReturnFloat() const { return return_float_; }
    double GetReturnDouble() const { return return_double_; }
    
    // Performance metrics
    uint64_t GetInstructionsExecuted() const { return instructions_executed_; }
    double GetExecutionTimeMs() const { return execution_time_ms_; }
    double GetOpsPerSecond() const { return ops_per_second_; }
    
    // Thread context access
    ThreadContext& GetContext() { return context_; }
    
private:
    // Core execution engine - optimized for performance
    void ExecuteLoop();
    
    // Instruction handlers - each optimized for maximum throughput
    void HandleNop();
    void HandleLdcI4();
    void HandleLdcR4();
    void HandleLdcR8();
    void HandleDup();
    void HandlePop();
    void HandleAdd();
    void HandleSub();
    void HandleMul();
    void HandleDiv();
    void HandleRem();
    void HandleAnd();
    void HandleOr();
    void HandleXor();
    void HandleShl();
    void HandleShr();
    void HandleNeg();
    void HandleNot();
    void HandleCeq();
    void HandleCgt();
    void HandleClt();
    void HandleBr();
    void HandleBrfalse();
    void HandleBrtrue();
    void HandleBeq();
    void HandleBge();
    void HandleBgt();
    void HandleBle();
    void HandleBlt();
    void HandleCall();
    void HandleRet();
    void HandleBox();
    void HandleUnbox();
    void HandleNewarr();
    void HandleLdlen();
    void HandleLdelem();
    void HandleStelem();
    void HandleLdfld();
    void HandleStfld();
    void HandleLdsfld();
    void HandleStsfld();
    void HandleNewobj();
    void HandleCastclass();
    void HandleIsinst();
    void HandleThrow();
    void HandleRethrow();
    void HandleLeave();
    void HandleEndfinally();
    
    // Inline cache operations
    bool CheckType(uint32_t type_id, InlineCache& cache);
    void RecordTypeHit(uint32_t type_id, InlineCache& cache);
    void RecordTypeMiss(InlineCache& cache);
    
    // Exception handling
    void PushExceptionFrame(uint32_t catch_pc, uint32_t finally_pc, uint32_t filter_pc);
    void PopExceptionFrame();
    void HandleException(void* exception);
    
    // Performance optimizations
    void OptimizeBranch(uint32_t target);
    void RecordBranchMispredict();
    void RecordCacheHit();
    void RecordCacheMiss();
    
    // Metadata access
    const Il2CppMethodDefinition* GetMethodDef(uint32_t index) const;
    const Il2CppTypeDefinition* GetTypeDef(uint32_t index) const;
    const char* GetString(uint32_t index) const;
    
    // References
    TypeSystem& type_system_;
    MemoryManager& memory_;
    InterpreterConfig config_;
    
    // Execution context - thread-local for performance
    ThreadContext context_;
    
    // State
    const MethodBody* current_method_ = nullptr;
    bool running_ = false;
    
    // Return values
    void* return_value_ = nullptr;
    int32_t return_int_ = 0;
    float return_float_ = 0.0f;
    double return_double_ = 0.0;
    
    // Native function registry
    std::unordered_map<std::string, NativeFunc> native_functions_;
    
    // Exception handling
    ExceptionFrame* exception_stack_ = nullptr;
    
    // Performance counters
    uint64_t instructions_executed_ = 0;
    uint64_t branch_mispredicts_ = 0;
    uint64_t cache_hits_ = 0;
    uint64_t cache_misses_ = 0;
    double execution_time_ms_ = 0.0;
    double ops_per_second_ = 0.0;
    
    // Dispatch table - function pointers for maximum performance
    using HandlerFunc = void (HighPerformanceInterpreter::*)();
    std::array<HandlerFunc, 256> dispatch_table_;
    
    // Initialize dispatch table
    void InitDispatchTable();
};

} // namespace hotc
