#pragma once

#include "platform.h"
#include "runtime.h"
#include "metadata.h"
#include "il_parser.h"
#include "type_system.h"
#include "memory.h"
#include <vector>
#include <functional>
#include <cstring>
#include <array>
#include <atomic>

namespace hotc {

using NativeFunc = void (*)(void* obj, void** args, void* result);

// Method call context for stack frames
struct CallFrame {
    const MethodBody* method;
    uint32_t pc;
    uint32_t stack_base;
    uint32_t local_base;
    void* return_addr;
    uint32_t arg_count;
    void** args;
    CallFrame* prev;
};

// Register instruction set
enum class RegOpCode : uint16_t {
    Nop = 0,
    Ldc_I4, Ldc_I4_S, Ldc_I4_M1, Ldc_R4, Ldc_R8,
    Ldloc, Stloc, Ldarg, Starg,
    Add_I4, Add_R4, Add_R8,
    Sub_I4, Sub_R4, Sub_R8,
    Mul_I4, Mul_R4, Mul_R8,
    Div_I4, Div_R4, Div_R8,
    Rem_I4, Rem_R4, Rem_R8,
    And_I4, Or_I4, Xor_I4,
    Shl_I4, Shr_I4, Shr_Un_I4,
    Neg_I4, Neg_R4, Neg_R8,
    Not_I4,
    Ceq_I4, Ceq_R4, Ceq_R8,
    Cgt_I4, Cgt_R4, Cgt_R8,
    Clt_I4, Clt_R4, Clt_R8,
    Br, Brfalse, Brtrue,
    Beq, Bge, Bgt, Ble, Blt,
    Beq_R4, Bge_R4, Bgt_R4, Ble_R4, Blt_R4,
    Box, Unbox, Newarr, Ldlen,
    Ldelem_I4, Stelem_I4, Ldelem_R4, Stelem_R4,
    Ldelem_Ref, Stelem_Ref,
    Ldfld, Stfld, Ldsfld, Stsfld,
    Call, Ret,
    Newobj, Castclass, Isinst,
    Throw, Rethrow,
    Ldnull, Dup, Pop,
    Ldstr, Ldtoken,
};

// Register instruction
struct RegInstruction {
    RegOpCode opcode;
    uint8_t dst;
    uint8_t src1;
    uint8_t src2;
    uint32_t operand;
};

// Complete interpreter with call dispatch and register compilation
class CompleteInterpreter {
public:
    CompleteInterpreter(TypeSystem& types, MemoryManager& memory);
    ~CompleteInterpreter() = default;

    // Execute a method with full call support
    void Execute(const MethodBody& method, void** args = nullptr, uint32_t arg_count = 0);

    // Register native function
    void RegisterNative(const std::string& name, NativeFunc func);

    // Return values
    int32_t GetReturnInt() const { return ret_i32_; }
    float GetReturnFloat() const { return ret_f32_; }
    double GetReturnDouble() const { return ret_f64_; }
    void* GetReturnObj() const { return ret_obj_; }

    // Register compiler: IL → register instructions
    std::vector<RegInstruction> CompileToRegister(const MethodBody& method);

    // Type checking
    bool IsInstanceOf(uint32_t obj_type, uint32_t target_type) const;

    // Get method body by index (for Call dispatch)
    const MethodBody* GetMethodBody(uint32_t method_index) const;
    void SetMethodBody(uint32_t method_index, const MethodBody* body);

private:
    // Execution loop
    void ExecuteLoop();

    // Register-compiled execution
    void ExecuteRegister(const std::vector<RegInstruction>& code);

    // Stack operations
    void PushI32(int32_t v);
    int32_t PopI32();
    void PushU32(uint32_t v);
    uint32_t PopU32();
    void PushF32(float v);
    float PopF32();
    void PushF64(double v);
    double PopF64();
    void PushObj(void* v);
    void* PopObj();

    // Call support
    void PushFrame(const MethodBody* method, uint32_t pc, uint32_t stack_base, void* ret_addr);
    void PopFrame();
    void CallMethod(uint32_t method_index, void** args, uint32_t arg_count);

    // Field access
    void* GetFieldAddr(void* obj, uint32_t field_offset);
    void* GetStaticField(uint32_t field_index);

    // Type operations
    bool IsSubclass(uint32_t child, uint32_t parent) const;

    // References
    TypeSystem& types_;
    MemoryManager& memory_;

    // Execution stacks
    std::vector<int32_t> i32_stack_;
    std::vector<uint32_t> u32_stack_;
    std::vector<float> f32_stack_;
    std::vector<double> f64_stack_;
    std::vector<void*> obj_stack_;

    // Local variables
    std::vector<void*> locals_;

    // Call frame stack
    CallFrame* frame_ = nullptr;
    CallFrame frames_[64];
    uint32_t frame_depth_ = 0;

    // Current execution state
    const MethodBody* current_method_ = nullptr;
    uint32_t pc_ = 0;
    bool running_ = false;

    // Return values
    int32_t ret_i32_ = 0;
    float ret_f32_ = 0;
    double ret_f64_ = 0;
    void* ret_obj_ = nullptr;

    // Native functions
    std::unordered_map<std::string, NativeFunc> native_funcs_;

    // Method bodies for call dispatch
    std::unordered_map<uint32_t, const MethodBody*> method_bodies_;

    // Generic instantiation cache
    struct GenericKey {
        uint32_t method_index;
        std::vector<uint32_t> type_args;
        bool operator==(const GenericKey& o) const;
    };
    struct GenericKeyHash { size_t operator()(const GenericKey& k) const; };
    std::unordered_map<GenericKey, uint32_t, GenericKeyHash> generic_cache_;

    // Type hierarchy for IsSubclass
    std::unordered_map<uint32_t, uint32_t> parent_type_;
};

} // namespace hotc
