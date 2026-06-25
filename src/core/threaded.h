#pragma once

#include "il_parser.h"
#include "type_system.h"
#include "memory.h"
#include <vector>
#include <unordered_map>
#include <functional>

namespace hotc {

using NativeFunc = std::function<void* (void** args, uint32_t arg_count)>;

class ThreadedInterpreter {
public:
    ThreadedInterpreter(TypeSystem& types, MemoryManager& memory);
    ~ThreadedInterpreter() = default;

    void Execute(const MethodBody& method);
    void Execute(const MethodBody& method, void** args, uint32_t arg_count);

    void RegisterNativeFunction(const std::string& name, NativeFunc func);

    void* GetReturnValue() const { return return_value_; }
    uint32_t GetReturnInt() const { return return_int_; }
    float GetReturnFloat() const { return return_float_; }
    double GetReturnDouble() const { return return_double_; }

private:
    using ThreadFunc = void (ThreadedInterpreter::*)();

    void InitDispatchTable();
    void Dispatch(OpCode opcode);

    void ExecNop();
    void ExecLdarg();
    void ExecStarg();
    void ExecLdloc();
    void ExecStloc();
    void ExecLdc_I4();
    void ExecLdc_R4();
    void ExecLdc_R8();
    void ExecDup();
    void ExecPop();
    void ExecAdd();
    void ExecSub();
    void ExecMul();
    void ExecDiv();
    void ExecRem();
    void ExecAnd();
    void ExecOr();
    void ExecXor();
    void ExecShl();
    void ExecShr();
    void ExecNeg();
    void ExecNot();
    void ExecCeq();
    void ExecCgt();
    void ExecClt();
    void ExecBr();
    void ExecBrfalse();
    void ExecBrtrue();
    void ExecCall();
    void ExecRet();
    void ExecBox();
    void ExecNewarr();
    void ExecLdlen();
    void ExecLdelem();
    void ExecStelem();
    void ExecLdfld();
    void ExecStfld();
    void ExecLdsfld();
    void ExecStsfld();
    void ExecNewobj();
    void ExecCastclass();
    void ExecIsinst();
    void ExecThrow();

    void Push(void* value);
    void* Pop();
    void PushInt(uint32_t value);
    uint32_t PopInt();
    void PushFloat(float value);
    float PopFloat();

    TypeSystem& type_system_;
    MemoryManager& memory_;

    std::vector<void*> stack_;
    std::vector<void*> locals_;
    std::vector<uint32_t> int_stack_;
    std::vector<float> float_stack_;
    std::vector<double> double_stack_;

    uint32_t pc_ = 0;
    bool running_ = false;

    void* return_value_ = nullptr;
    uint32_t return_int_ = 0;
    float return_float_ = 0.0f;
    double return_double_ = 0.0;

    std::unordered_map<std::string, NativeFunc> native_functions_;
    const MethodBody* current_method_ = nullptr;

    ThreadFunc dispatch_table_[256];
};

} // namespace hotc
