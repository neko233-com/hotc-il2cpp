#pragma once

#include "platform.h"
#include "il_parser.h"
#include "type_system.h"
#include "memory.h"
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <string>
#include <functional>

namespace hotc {

// Forward declarations
struct MethodBody;

// Native function type
using NativeFunc = void (*)(void* obj, void** args, void* result);

// Method call stack frame
struct CallFrame {
    const MethodBody* method;
    uint32_t pc;
    uint32_t stack_base;
    void* return_addr;
    CallFrame* prev;
};

// Register instruction opcodes
enum class RegOpCode : uint16_t {
    Nop = 0,
    Ldc_I4, Ldc_R4, Ldc_R8,
    Ldloc, Stloc, Ldarg, Starg,
    Add_I4, Sub_I4, Mul_I4, Div_I4, Rem_I4,
    And_I4, Or_I4, Xor_I4, Shl_I4, Shr_I4,
    Neg_I4, Not_I4,
    Ceq_I4, Cgt_I4, Clt_I4,
    Br, Brfalse, Brtrue,
    Beq, Bge, Bgt, Ble, Blt,
    Box, Newarr, Ldlen,
    Ldelem_I4, Stelem_I4, Ldelem_R4, Stelem_R4,
    Ldelem_Ref, Stelem_Ref,
    Ldfld, Stfld, Ldsfld, Stsfld,
    Call, Ret, Newobj,
    Castclass, Isinst, Throw, Ldnull, Dup, Pop, Ldstr,
};

// Register instruction
struct RegInstruction {
    RegOpCode opcode;
    uint8_t dst;
    uint8_t src1;
    uint8_t src2;
    uint32_t operand;
};

// Complete IL interpreter with full Call dispatch, float ops, generics support
class CompleteInterpreter {
public:
    CompleteInterpreter(TypeSystem& types, MemoryManager& memory);

    void Execute(const MethodBody& method, void** args = nullptr, uint32_t arg_count = 0);
    void RegisterNative(const std::string& name, NativeFunc func);

    int32_t GetReturnInt() const { return ret_i32_; }
    float GetReturnFloat() const { return ret_f32_; }
    double GetReturnDouble() const { return ret_f64_; }
    void* GetReturnObj() const { return ret_obj_; }

    std::vector<RegInstruction> CompileToRegister(const MethodBody& method);
    void SetMethodBody(uint32_t idx, const MethodBody* body);
    const MethodBody* GetMethodBody(uint32_t idx) const;

private:
    void ExecuteLoop();

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

    void PushFrame(const MethodBody* method, uint32_t pc, uint32_t stack_base, void* ret_addr);
    void PopFrame();
    void CallMethod(uint32_t method_index, void** args, uint32_t arg_count);

    TypeSystem& types_;
    MemoryManager& memory_;

    std::vector<int32_t> i32_stack_;
    std::vector<uint32_t> u32_stack_;
    std::vector<float> f32_stack_;
    std::vector<double> f64_stack_;
    std::vector<void*> obj_stack_;
    std::vector<void*> locals_;

    CallFrame* frame_ = nullptr;
    CallFrame frames_[64];
    uint32_t frame_depth_ = 0;

    const MethodBody* current_method_ = nullptr;
    uint32_t pc_ = 0;
    bool running_ = false;

    int32_t ret_i32_ = 0;
    float ret_f32_ = 0;
    double ret_f64_ = 0;
    void* ret_obj_ = nullptr;

    std::unordered_map<std::string, NativeFunc> native_funcs_;
    std::unordered_map<uint32_t, const MethodBody*> method_bodies_;
    std::unordered_map<uint32_t, uint32_t> parent_type_;
};

} // namespace hotc
