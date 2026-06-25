#pragma once

#include "il_parser.h"
#include "type_system.h"
#include "memory.h"
#include <vector>
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
    int32_t GetReturnInt() const { return return_int_; }
    float GetReturnFloat() const { return return_float_; }
    double GetReturnDouble() const { return return_double_; }

private:
    using ThreadFunc = void (ThreadedInterpreter::*)();

    void InitDispatchTable();
    void Dispatch(OpCode opcode);

    void ExecNop();
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

    void PushObject(void* value);
    void* PopObject();
    void PushInt32(int32_t value);
    int32_t PopInt32();
    void PushUInt32(uint32_t value);
    uint32_t PopUInt32();
    void PushFloat(float value);
    float PopFloat();
    void PushDouble(double value);
    double PopDouble();

    TypeSystem& type_system_;
    MemoryManager& memory_;

    std::vector<void*> object_stack_;
    std::vector<int32_t> int32_stack_;
    std::vector<uint32_t> uint32_stack_;
    std::vector<int64_t> int64_stack_;
    std::vector<float> float_stack_;
    std::vector<double> double_stack_;
    std::vector<void*> locals_;

    uint32_t pc_ = 0;
    bool running_ = false;

    void* return_value_ = nullptr;
    int32_t return_int_ = 0;
    float return_float_ = 0.0f;
    double return_double_ = 0.0;

    std::unordered_map<std::string, NativeFunc> native_functions_;
    const MethodBody* current_method_ = nullptr;

    ThreadFunc dispatch_table_[512];
};

} // namespace hotc
