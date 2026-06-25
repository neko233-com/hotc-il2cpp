#pragma once

#include "il_parser.h"
#include "type_system.h"
#include "memory.h"
#include <vector>
#include <stack>
#include <unordered_map>
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
};

} // namespace hotc
