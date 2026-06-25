#pragma once

#include "il_parser.h"
#include "type_system.h"
#include "memory.h"
#include <vector>
#include <functional>

namespace hotc {

using NativeFunc = std::function<void* (void** args, uint32_t arg_count)>;

class Interpreter {
public:
    Interpreter(TypeSystem& types, MemoryManager& memory);
    ~Interpreter() = default;

    void Execute(const MethodBody& method);
    void Execute(const MethodBody& method, void** args, uint32_t arg_count);

    void RegisterNativeFunction(const std::string& name, NativeFunc func);

    void* GetReturnValue() const { return return_value_; }
    uint32_t GetReturnInt() const { return return_int_; }
    float GetReturnFloat() const { return return_float_; }
    double GetReturnDouble() const { return return_double_; }

private:
    void ExecuteInstruction(const Instruction& instr);

    void PushObject(void* value);
    void* PopObject();
    void PushInt32(int32_t value);
    int32_t PopInt32();
    void PushUInt32(uint32_t value);
    uint32_t PopUInt32();
    void PushInt64(int64_t value);
    int64_t PopInt64();
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
};

} // namespace hotc
