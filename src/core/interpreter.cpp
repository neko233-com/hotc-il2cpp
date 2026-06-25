#include "interpreter.h"
#include <cstring>
#include <stdexcept>

namespace hotc {

Interpreter::Interpreter(TypeSystem& types, MemoryManager& memory)
    : type_system_(types), memory_(memory) {
    stack_.reserve(1024);
    locals_.reserve(256);
    int_stack_.reserve(1024);
    float_stack_.reserve(256);
    double_stack_.reserve(256);
}

void Interpreter::Execute(const MethodBody& method) {
    Execute(method, nullptr, 0);
}

void Interpreter::Execute(const MethodBody& method, void** args, uint32_t arg_count) {
    current_method_ = &method;
    pc_ = 0;
    running_ = true;

    locals_.resize(method.max_locals);
    stack_.clear();
    int_stack_.clear();
    float_stack_.clear();
    double_stack_.clear();

    while (running_ && pc_ < method.instructions.size()) {
        const auto& instr = method.instructions[pc_];
        ExecuteInstruction(instr);
        pc_++;
    }
}

void Interpreter::ExecuteInstruction(const Instruction& instr) {
    switch (instr.opcode) {
        case OpCode::Nop:
            break;

        case OpCode::Ldc_I4_0:
            PushInt(0);
            break;
        case OpCode::Ldc_I4_1:
            PushInt(1);
            break;
        case OpCode::Ldc_I4_2:
            PushInt(2);
            break;
        case OpCode::Ldc_I4_3:
            PushInt(3);
            break;
        case OpCode::Ldc_I4_4:
            PushInt(4);
            break;
        case OpCode::Ldc_I4_5:
            PushInt(5);
            break;
        case OpCode::Ldc_I4_6:
            PushInt(6);
            break;
        case OpCode::Ldc_I4_7:
            PushInt(7);
            break;
        case OpCode::Ldc_I4_8:
            PushInt(8);
            break;
        case OpCode::Ldc_I4_M1:
            PushInt(static_cast<uint32_t>(-1));
            break;
        case OpCode::Ldc_I4:
        case OpCode::Ldc_I4_S:
            PushInt(instr.operand);
            break;

        case OpCode::Ldc_R4: {
            float value;
            std::memcpy(&value, &instr.operand, sizeof(float));
            PushFloat(value);
            break;
        }

        case OpCode::Ldloc_0:
            Push(locals_[0]);
            break;
        case OpCode::Ldloc_1:
            Push(locals_[1]);
            break;
        case OpCode::Ldloc_2:
            Push(locals_[2]);
            break;
        case OpCode::Ldloc_3:
            Push(locals_[3]);
            break;
        case OpCode::Ldloc_S:
        case OpCode::Ldloc:
            Push(locals_[instr.operand]);
            break;

        case OpCode::Stloc_0:
            locals_[0] = Pop();
            break;
        case OpCode::Stloc_1:
            locals_[1] = Pop();
            break;
        case OpCode::Stloc_2:
            locals_[2] = Pop();
            break;
        case OpCode::Stloc_3:
            locals_[3] = Pop();
            break;
        case OpCode::Stloc_S:
        case OpCode::Stloc:
            locals_[instr.operand] = Pop();
            break;

        case OpCode::Ldarg_0:
        case OpCode::Ldarg_1:
        case OpCode::Ldarg_2:
        case OpCode::Ldarg_3:
        case OpCode::Ldarg_S:
        case OpCode::Ldarg:
            // Args handled via stack in real implementation
            break;

        case OpCode::Dup: {
            void* value = Pop();
            Push(value);
            Push(value);
            break;
        }

        case OpCode::Pop:
            Pop();
            break;

        case OpCode::Add: {
            uint32_t b = PopInt();
            uint32_t a = PopInt();
            PushInt(a + b);
            break;
        }

        case OpCode::Sub: {
            uint32_t b = PopInt();
            uint32_t a = PopInt();
            PushInt(a - b);
            break;
        }

        case OpCode::Mul: {
            uint32_t b = PopInt();
            uint32_t a = PopInt();
            PushInt(a * b);
            break;
        }

        case OpCode::Div: {
            uint32_t b = PopInt();
            uint32_t a = PopInt();
            PushInt(a / b);
            break;
        }

        case OpCode::Rem: {
            uint32_t b = PopInt();
            uint32_t a = PopInt();
            PushInt(a % b);
            break;
        }

        case OpCode::And: {
            uint32_t b = PopInt();
            uint32_t a = PopInt();
            PushInt(a & b);
            break;
        }

        case OpCode::Or: {
            uint32_t b = PopInt();
            uint32_t a = PopInt();
            PushInt(a | b);
            break;
        }

        case OpCode::Xor: {
            uint32_t b = PopInt();
            uint32_t a = PopInt();
            PushInt(a ^ b);
            break;
        }

        case OpCode::Shl: {
            uint32_t b = PopInt();
            uint32_t a = PopInt();
            PushInt(a << b);
            break;
        }

        case OpCode::Shr: {
            uint32_t b = PopInt();
            int32_t a = static_cast<int32_t>(PopInt());
            PushInt(static_cast<uint32_t>(a >> b));
            break;
        }

        case OpCode::Neg: {
            int32_t value = static_cast<int32_t>(PopInt());
            PushInt(static_cast<uint32_t>(-value));
            break;
        }

        case OpCode::Not: {
            uint32_t value = PopInt();
            PushInt(~value);
            break;
        }

        case OpCode::Ceq: {
            uint32_t b = PopInt();
            uint32_t a = PopInt();
            PushInt(a == b ? 1 : 0);
            break;
        }

        case OpCode::Cgt: {
            uint32_t b = PopInt();
            uint32_t a = PopInt();
            PushInt(a > b ? 1 : 0);
            break;
        }

        case OpCode::Clt: {
            uint32_t b = PopInt();
            uint32_t a = PopInt();
            PushInt(a < b ? 1 : 0);
            break;
        }

        case OpCode::Br:
        case OpCode::Br_S:
            pc_ = instr.operand - 1; // -1 because pc_++ will execute
            break;

        case OpCode::Brfalse:
        case OpCode::Brfalse_S: {
            uint32_t value = PopInt();
            if (value == 0) {
                pc_ = instr.operand - 1;
            }
            break;
        }

        case OpCode::Brtrue:
        case OpCode::Brtrue_S: {
            uint32_t value = PopInt();
            if (value != 0) {
                pc_ = instr.operand - 1;
            }
            break;
        }

        case OpCode::Call: {
            // Find and call the method
            // In real implementation, this would look up the method table
            break;
        }

        case OpCode::Ret:
            running_ = false;
            break;

        case OpCode::Ldnull:
            Push(nullptr);
            break;

        case OpCode::Throw: {
            void* exception = Pop();
            // Handle exception
            running_ = false;
            break;
        }

        default:
            throw std::runtime_error("Unsupported opcode");
    }
}

void Interpreter::Push(void* value) {
    stack_.push_back(value);
}

void* Interpreter::Pop() {
    if (stack_.empty()) {
        throw std::runtime_error("Stack underflow");
    }
    void* value = stack_.back();
    stack_.pop_back();
    return value;
}

void Interpreter::PushInt(uint32_t value) {
    int_stack_.push_back(value);
}

uint32_t Interpreter::PopInt() {
    if (int_stack_.empty()) {
        throw std::runtime_error("Int stack underflow");
    }
    uint32_t value = int_stack_.back();
    int_stack_.pop_back();
    return value;
}

void Interpreter::PushFloat(float value) {
    float_stack_.push_back(value);
}

float Interpreter::PopFloat() {
    if (float_stack_.empty()) {
        throw std::runtime_error("Float stack underflow");
    }
    float value = float_stack_.back();
    float_stack_.pop_back();
    return value;
}

void Interpreter::RegisterNativeFunction(const std::string& name, NativeFunc func) {
    native_functions_[name] = std::move(func);
}

} // namespace hotc
