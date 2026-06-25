#include "interpreter.h"
#include <cstring>
#include <stdexcept>

namespace hotc {

Interpreter::Interpreter(TypeSystem& types, MemoryManager& memory)
    : type_system_(types), memory_(memory) {
    object_stack_.reserve(1024);
    int32_stack_.reserve(1024);
    uint32_stack_.reserve(1024);
    int64_stack_.reserve(256);
    float_stack_.reserve(256);
    double_stack_.reserve(256);
    locals_.reserve(256);
}

void Interpreter::Execute(const MethodBody& method) {
    Execute(method, nullptr, 0);
}

void Interpreter::Execute(const MethodBody& method, void** args, uint32_t arg_count) {
    current_method_ = &method;
    pc_ = 0;
    running_ = true;

    locals_.resize(method.max_locals, nullptr);
    object_stack_.clear();
    int32_stack_.clear();
    uint32_stack_.clear();
    int64_stack_.clear();
    float_stack_.clear();
    double_stack_.clear();

    if (args && arg_count > 0) {
        for (uint32_t i = 0; i < arg_count; i++) {
            locals_[i] = args[i];
        }
    }

    while (running_ && pc_ < static_cast<uint32_t>(method.instructions.size())) {
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
            PushInt32(0);
            break;
        case OpCode::Ldc_I4_1:
            PushInt32(1);
            break;
        case OpCode::Ldc_I4_2:
            PushInt32(2);
            break;
        case OpCode::Ldc_I4_3:
            PushInt32(3);
            break;
        case OpCode::Ldc_I4_4:
            PushInt32(4);
            break;
        case OpCode::Ldc_I4_5:
            PushInt32(5);
            break;
        case OpCode::Ldc_I4_6:
            PushInt32(6);
            break;
        case OpCode::Ldc_I4_7:
            PushInt32(7);
            break;
        case OpCode::Ldc_I4_8:
            PushInt32(8);
            break;
        case OpCode::Ldc_I4_M1:
            PushInt32(-1);
            break;
        case OpCode::Ldc_I4:
        case OpCode::Ldc_I4_S:
            PushInt32(static_cast<int32_t>(instr.operand));
            break;

        case OpCode::Ldc_R4: {
            float value;
            std::memcpy(&value, &instr.operand, sizeof(float));
            PushFloat(value);
            break;
        }

        case OpCode::Ldc_R8: {
            uint32_t low = instr.operand;
            uint32_t high = (pc_ + 1 < static_cast<uint32_t>(current_method_->instructions.size())) 
                ? current_method_->instructions[pc_ + 1].operand : 0;
            uint64_t raw = (static_cast<uint64_t>(high) << 32) | low;
            double value;
            std::memcpy(&value, &raw, sizeof(double));
            PushDouble(value);
            break;
        }

        case OpCode::LDloc_0:
            if (locals_.size() > 0) PushObject(locals_[0]);
            break;
        case OpCode::LDloc_1:
            if (locals_.size() > 1) PushObject(locals_[1]);
            break;
        case OpCode::LDloc_2:
            if (locals_.size() > 2) PushObject(locals_[2]);
            break;
        case OpCode::LDloc_3:
            if (locals_.size() > 3) PushObject(locals_[3]);
            break;
        case OpCode::LDloc_S:
        case OpCode::LDloc:
            if (instr.operand < locals_.size()) PushObject(locals_[instr.operand]);
            break;

        case OpCode::STloc_0:
            if (locals_.size() > 0) locals_[0] = PopObject();
            break;
        case OpCode::STloc_1:
            if (locals_.size() > 1) locals_[1] = PopObject();
            break;
        case OpCode::STloc_2:
            if (locals_.size() > 2) locals_[2] = PopObject();
            break;
        case OpCode::STloc_3:
            if (locals_.size() > 3) locals_[3] = PopObject();
            break;
        case OpCode::STloc_S:
        case OpCode::STloc:
            if (instr.operand < locals_.size()) locals_[instr.operand] = PopObject();
            break;

        case OpCode::LDarg_0:
        case OpCode::LDarg_1:
        case OpCode::LDarg_2:
        case OpCode::LDarg_3:
        case OpCode::LDarg_S:
        case OpCode::LDarg:
            break;

        case OpCode::Dup: {
            if (!int32_stack_.empty()) {
                int32_t value = PopInt32();
                PushInt32(value);
                PushInt32(value);
            } else {
                void* value = PopObject();
                PushObject(value);
                PushObject(value);
            }
            break;
        }

        case OpCode::Pop:
            PopObject();
            break;

        case OpCode::Add: {
            int32_t b = PopInt32();
            int32_t a = PopInt32();
            PushInt32(a + b);
            break;
        }

        case OpCode::Sub: {
            int32_t b = PopInt32();
            int32_t a = PopInt32();
            PushInt32(a - b);
            break;
        }

        case OpCode::Mul: {
            int32_t b = PopInt32();
            int32_t a = PopInt32();
            PushInt32(a * b);
            break;
        }

        case OpCode::Div: {
            int32_t b = PopInt32();
            int32_t a = PopInt32();
            PushInt32(a / b);
            break;
        }

        case OpCode::Div_Un: {
            uint32_t b = PopUInt32();
            uint32_t a = PopUInt32();
            PushUInt32(a / b);
            break;
        }

        case OpCode::Rem: {
            int32_t b = PopInt32();
            int32_t a = PopInt32();
            PushInt32(a % b);
            break;
        }

        case OpCode::Rem_Un: {
            uint32_t b = PopUInt32();
            uint32_t a = PopUInt32();
            PushUInt32(a % b);
            break;
        }

        case OpCode::And: {
            int32_t b = PopInt32();
            int32_t a = PopInt32();
            PushInt32(a & b);
            break;
        }

        case OpCode::Or: {
            int32_t b = PopInt32();
            int32_t a = PopInt32();
            PushInt32(a | b);
            break;
        }

        case OpCode::Xor: {
            int32_t b = PopInt32();
            int32_t a = PopInt32();
            PushInt32(a ^ b);
            break;
        }

        case OpCode::Shl: {
            int32_t b = PopInt32();
            int32_t a = PopInt32();
            PushInt32(a << b);
            break;
        }

        case OpCode::Shr: {
            int32_t b = PopInt32();
            int32_t a = PopInt32();
            PushInt32(a >> b);
            break;
        }

        case OpCode::Shr_Un: {
            uint32_t b = PopUInt32();
            uint32_t a = PopUInt32();
            PushUInt32(a >> b);
            break;
        }

        case OpCode::Neg: {
            int32_t value = PopInt32();
            PushInt32(-value);
            break;
        }

        case OpCode::Not: {
            int32_t value = PopInt32();
            PushInt32(~value);
            break;
        }

        case OpCode::Ceq: {
            int32_t b = PopInt32();
            int32_t a = PopInt32();
            PushInt32(a == b ? 1 : 0);
            break;
        }

        case OpCode::Cgt: {
            int32_t b = PopInt32();
            int32_t a = PopInt32();
            PushInt32(a > b ? 1 : 0);
            break;
        }

        case OpCode::Clt: {
            int32_t b = PopInt32();
            int32_t a = PopInt32();
            PushInt32(a < b ? 1 : 0);
            break;
        }

        case OpCode::Br:
        case OpCode::Br_S:
            pc_ = instr.operand - 1;
            break;

        case OpCode::Brfalse:
        case OpCode::Brfalse_S: {
            int32_t value = PopInt32();
            if (value == 0) {
                pc_ = instr.operand - 1;
            }
            break;
        }

        case OpCode::Brtrue:
        case OpCode::Brtrue_S: {
            int32_t value = PopInt32();
            if (value != 0) {
                pc_ = instr.operand - 1;
            }
            break;
        }

        case OpCode::Beq:
        case OpCode::Beq_S: {
            int32_t b = PopInt32();
            int32_t a = PopInt32();
            if (a == b) {
                pc_ = instr.operand - 1;
            }
            break;
        }

        case OpCode::Bge:
        case OpCode::Bge_S: {
            int32_t b = PopInt32();
            int32_t a = PopInt32();
            if (a >= b) {
                pc_ = instr.operand - 1;
            }
            break;
        }

        case OpCode::Bgt:
        case OpCode::Bgt_S: {
            int32_t b = PopInt32();
            int32_t a = PopInt32();
            if (a > b) {
                pc_ = instr.operand - 1;
            }
            break;
        }

        case OpCode::Ble:
        case OpCode::Ble_S: {
            int32_t b = PopInt32();
            int32_t a = PopInt32();
            if (a <= b) {
                pc_ = instr.operand - 1;
            }
            break;
        }

        case OpCode::Blt:
        case OpCode::Blt_S: {
            int32_t b = PopInt32();
            int32_t a = PopInt32();
            if (a < b) {
                pc_ = instr.operand - 1;
            }
            break;
        }

        case OpCode::Bne_Un:
        case OpCode::Bne_Un_S: {
            uint32_t b = PopUInt32();
            uint32_t a = PopUInt32();
            if (a != b) {
                pc_ = instr.operand - 1;
            }
            break;
        }

        case OpCode::Bge_Un:
        case OpCode::Bge_Un_S: {
            uint32_t b = PopUInt32();
            uint32_t a = PopUInt32();
            if (a >= b) {
                pc_ = instr.operand - 1;
            }
            break;
        }

        case OpCode::Bgt_Un:
        case OpCode::Bgt_Un_S: {
            uint32_t b = PopUInt32();
            uint32_t a = PopUInt32();
            if (a > b) {
                pc_ = instr.operand - 1;
            }
            break;
        }

        case OpCode::Ble_Un:
        case OpCode::Ble_Un_S: {
            uint32_t b = PopUInt32();
            uint32_t a = PopUInt32();
            if (a <= b) {
                pc_ = instr.operand - 1;
            }
            break;
        }

        case OpCode::Blt_Un:
        case OpCode::Blt_Un_S: {
            uint32_t b = PopUInt32();
            uint32_t a = PopUInt32();
            if (a < b) {
                pc_ = instr.operand - 1;
            }
            break;
        }

        case OpCode::Call: {
            break;
        }

        case OpCode::Ret:
            if (!int32_stack_.empty()) {
                return_int_ = PopInt32();
            }
            running_ = false;
            break;

        case OpCode::LDnull: {
            PushObject(nullptr);
            break;
        }

        case OpCode::Box: {
            PopObject();
            break;
        }

        case OpCode::Newarr: {
            uint32_t count = PopUInt32();
            void* arr = memory_.AllocateArray(4, count);
            PushObject(arr);
            break;
        }

        case OpCode::Ldlen: {
            void* arr = PopObject();
            uint32_t len = *static_cast<uint32_t*>(arr);
            PushUInt32(len);
            break;
        }

        case OpCode::Ldelem_I4:
        case OpCode::Ldelem_Any: {
            uint32_t index = PopUInt32();
            void* arr = PopObject();
            uint8_t* base = static_cast<uint8_t*>(arr) + sizeof(uint32_t);
            uint32_t* elements = reinterpret_cast<uint32_t*>(base);
            PushInt32(static_cast<int32_t>(elements[index]));
            break;
        }

        case OpCode::Stelem_I4:
        case OpCode::Stelem_Any: {
            int32_t value = PopInt32();
            uint32_t index = PopUInt32();
            void* arr = PopObject();
            uint8_t* base = static_cast<uint8_t*>(arr) + sizeof(uint32_t);
            uint32_t* elements = reinterpret_cast<uint32_t*>(base);
            elements[index] = static_cast<uint32_t>(value);
            break;
        }

        case OpCode::Ldfld: {
            void* obj = PopObject();
            if (obj) {
                uint32_t offset = instr.operand;
                void* field = static_cast<uint8_t*>(obj) + offset;
                PushObject(field);
            }
            break;
        }

        case OpCode::Stfld: {
            void* value = PopObject();
            void* obj = PopObject();
            if (obj) {
                uint32_t offset = instr.operand;
                void* field = static_cast<uint8_t*>(obj) + offset;
                std::memcpy(field, &value, sizeof(void*));
            }
            break;
        }

        case OpCode::Ldsfld: {
            PushObject(nullptr);
            break;
        }

        case OpCode::Stsfld: {
            PopObject();
            break;
        }

        case OpCode::Newobj: {
            PushObject(nullptr);
            break;
        }

        case OpCode::Castclass: {
            break;
        }

        case OpCode::Isinst: {
            break;
        }

        case OpCode::Throw: {
            PopObject();
            running_ = false;
            break;
        }

        default:
            break;
    }
}

void Interpreter::PushObject(void* value) {
    object_stack_.push_back(value);
}

void* Interpreter::PopObject() {
    if (object_stack_.empty()) {
        return nullptr;
    }
    void* value = object_stack_.back();
    object_stack_.pop_back();
    return value;
}

void Interpreter::PushInt32(int32_t value) {
    int32_stack_.push_back(value);
}

int32_t Interpreter::PopInt32() {
    if (int32_stack_.empty()) {
        return 0;
    }
    int32_t value = int32_stack_.back();
    int32_stack_.pop_back();
    return value;
}

void Interpreter::PushUInt32(uint32_t value) {
    uint32_stack_.push_back(value);
}

uint32_t Interpreter::PopUInt32() {
    if (uint32_stack_.empty()) {
        return 0;
    }
    uint32_t value = uint32_stack_.back();
    uint32_stack_.pop_back();
    return value;
}

void Interpreter::PushInt64(int64_t value) {
    int64_stack_.push_back(value);
}

int64_t Interpreter::PopInt64() {
    if (int64_stack_.empty()) {
        return 0;
    }
    int64_t value = int64_stack_.back();
    int64_stack_.pop_back();
    return value;
}

void Interpreter::PushFloat(float value) {
    float_stack_.push_back(value);
}

float Interpreter::PopFloat() {
    if (float_stack_.empty()) {
        return 0.0f;
    }
    float value = float_stack_.back();
    float_stack_.pop_back();
    return value;
}

void Interpreter::PushDouble(double value) {
    double_stack_.push_back(value);
}

double Interpreter::PopDouble() {
    if (double_stack_.empty()) {
        return 0.0;
    }
    double value = double_stack_.back();
    double_stack_.pop_back();
    return value;
}

void Interpreter::RegisterNativeFunction(const std::string& name, NativeFunc func) {
    native_functions_[name] = std::move(func);
}

} // namespace hotc
