#include "threaded.h"
#include <cstring>
#include <stdexcept>

namespace hotc {

ThreadedInterpreter::ThreadedInterpreter(TypeSystem& types, MemoryManager& memory)
    : type_system_(types), memory_(memory) {
    InitDispatchTable();
    object_stack_.reserve(1024);
    int32_stack_.reserve(1024);
    uint32_stack_.reserve(1024);
    int64_stack_.reserve(256);
    float_stack_.reserve(256);
    double_stack_.reserve(256);
    locals_.reserve(256);
}

void ThreadedInterpreter::InitDispatchTable() {
    for (int i = 0; i < 256; i++) {
        dispatch_table_[i] = &ThreadedInterpreter::ExecNop;
    }

    dispatch_table_[static_cast<uint8_t>(OpCode::Nop)] = &ThreadedInterpreter::ExecNop;
    dispatch_table_[static_cast<uint8_t>(OpCode::Ldc_I4)] = &ThreadedInterpreter::ExecLdc_I4;
    dispatch_table_[static_cast<uint8_t>(OpCode::Ldc_R4)] = &ThreadedInterpreter::ExecLdc_R4;
    dispatch_table_[static_cast<uint8_t>(OpCode::Dup)] = &ThreadedInterpreter::ExecDup;
    dispatch_table_[static_cast<uint8_t>(OpCode::Pop)] = &ThreadedInterpreter::ExecPop;
    dispatch_table_[static_cast<uint8_t>(OpCode::Add)] = &ThreadedInterpreter::ExecAdd;
    dispatch_table_[static_cast<uint8_t>(OpCode::Sub)] = &ThreadedInterpreter::ExecSub;
    dispatch_table_[static_cast<uint8_t>(OpCode::Mul)] = &ThreadedInterpreter::ExecMul;
    dispatch_table_[static_cast<uint8_t>(OpCode::Div)] = &ThreadedInterpreter::ExecDiv;
    dispatch_table_[static_cast<uint8_t>(OpCode::Rem)] = &ThreadedInterpreter::ExecRem;
    dispatch_table_[static_cast<uint8_t>(OpCode::And)] = &ThreadedInterpreter::ExecAnd;
    dispatch_table_[static_cast<uint8_t>(OpCode::Or)] = &ThreadedInterpreter::ExecOr;
    dispatch_table_[static_cast<uint8_t>(OpCode::Xor)] = &ThreadedInterpreter::ExecXor;
    dispatch_table_[static_cast<uint8_t>(OpCode::Shl)] = &ThreadedInterpreter::ExecShl;
    dispatch_table_[static_cast<uint8_t>(OpCode::Shr)] = &ThreadedInterpreter::ExecShr;
    dispatch_table_[static_cast<uint8_t>(OpCode::Neg)] = &ThreadedInterpreter::ExecNeg;
    dispatch_table_[static_cast<uint8_t>(OpCode::Not)] = &ThreadedInterpreter::ExecNot;
    dispatch_table_[static_cast<uint8_t>(OpCode::Ceq)] = &ThreadedInterpreter::ExecCeq;
    dispatch_table_[static_cast<uint8_t>(OpCode::Cgt)] = &ThreadedInterpreter::ExecCgt;
    dispatch_table_[static_cast<uint8_t>(OpCode::Clt)] = &ThreadedInterpreter::ExecClt;
    dispatch_table_[static_cast<uint8_t>(OpCode::Br)] = &ThreadedInterpreter::ExecBr;
    dispatch_table_[static_cast<uint8_t>(OpCode::Brfalse)] = &ThreadedInterpreter::ExecBrfalse;
    dispatch_table_[static_cast<uint8_t>(OpCode::Brtrue)] = &ThreadedInterpreter::ExecBrtrue;
    dispatch_table_[static_cast<uint8_t>(OpCode::Call)] = &ThreadedInterpreter::ExecCall;
    dispatch_table_[static_cast<uint8_t>(OpCode::Ret)] = &ThreadedInterpreter::ExecRet;
    dispatch_table_[static_cast<uint8_t>(OpCode::Box)] = &ThreadedInterpreter::ExecBox;
    dispatch_table_[static_cast<uint8_t>(OpCode::Newarr)] = &ThreadedInterpreter::ExecNewarr;
    dispatch_table_[static_cast<uint8_t>(OpCode::Ldlen)] = &ThreadedInterpreter::ExecLdlen;
    dispatch_table_[static_cast<uint8_t>(OpCode::Ldfld)] = &ThreadedInterpreter::ExecLdfld;
    dispatch_table_[static_cast<uint8_t>(OpCode::Stfld)] = &ThreadedInterpreter::ExecStfld;
    dispatch_table_[static_cast<uint8_t>(OpCode::Ldsfld)] = &ThreadedInterpreter::ExecLdsfld;
    dispatch_table_[static_cast<uint8_t>(OpCode::Stsfld)] = &ThreadedInterpreter::ExecStsfld;
    dispatch_table_[static_cast<uint8_t>(OpCode::Newobj)] = &ThreadedInterpreter::ExecNewobj;
    dispatch_table_[static_cast<uint8_t>(OpCode::Castclass)] = &ThreadedInterpreter::ExecCastclass;
    dispatch_table_[static_cast<uint8_t>(OpCode::Isinst)] = &ThreadedInterpreter::ExecIsinst;
    dispatch_table_[static_cast<uint8_t>(OpCode::Throw)] = &ThreadedInterpreter::ExecThrow;
}

void ThreadedInterpreter::Dispatch(OpCode opcode) {
    uint8_t idx = static_cast<uint8_t>(opcode);
    (this->*dispatch_table_[idx])();
}

void ThreadedInterpreter::Execute(const MethodBody& method) {
    Execute(method, nullptr, 0);
}

void ThreadedInterpreter::Execute(const MethodBody& method, void** args, uint32_t arg_count) {
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
        Dispatch(instr.opcode);
        pc_++;
    }
}

void ThreadedInterpreter::ExecNop() {}
void ThreadedInterpreter::ExecLdc_I4() { PushInt32(static_cast<int32_t>(current_method_->instructions[pc_].operand)); }
void ThreadedInterpreter::ExecLdc_R4() {
    float value;
    std::memcpy(&value, &current_method_->instructions[pc_].operand, sizeof(float));
    PushFloat(value);
}
void ThreadedInterpreter::ExecLdc_R8() {
    uint32_t low = current_method_->instructions[pc_].operand;
    uint32_t high = (pc_ + 1 < static_cast<uint32_t>(current_method_->instructions.size())) 
        ? current_method_->instructions[pc_ + 1].operand : 0;
    uint64_t raw = (static_cast<uint64_t>(high) << 32) | low;
    double value;
    std::memcpy(&value, &raw, sizeof(double));
    PushDouble(value);
}
void ThreadedInterpreter::ExecDup() {
    void* value = PopObject();
    PushObject(value);
    PushObject(value);
}
void ThreadedInterpreter::ExecPop() { PopObject(); }

void ThreadedInterpreter::ExecAdd() {
    int32_t b = PopInt32();
    int32_t a = PopInt32();
    PushInt32(a + b);
}

void ThreadedInterpreter::ExecSub() {
    int32_t b = PopInt32();
    int32_t a = PopInt32();
    PushInt32(a - b);
}

void ThreadedInterpreter::ExecMul() {
    int32_t b = PopInt32();
    int32_t a = PopInt32();
    PushInt32(a * b);
}

void ThreadedInterpreter::ExecDiv() {
    int32_t b = PopInt32();
    int32_t a = PopInt32();
    PushInt32(a / b);
}

void ThreadedInterpreter::ExecRem() {
    int32_t b = PopInt32();
    int32_t a = PopInt32();
    PushInt32(a % b);
}

void ThreadedInterpreter::ExecAnd() {
    int32_t b = PopInt32();
    int32_t a = PopInt32();
    PushInt32(a & b);
}

void ThreadedInterpreter::ExecOr() {
    int32_t b = PopInt32();
    int32_t a = PopInt32();
    PushInt32(a | b);
}

void ThreadedInterpreter::ExecXor() {
    int32_t b = PopInt32();
    int32_t a = PopInt32();
    PushInt32(a ^ b);
}

void ThreadedInterpreter::ExecShl() {
    int32_t b = PopInt32();
    int32_t a = PopInt32();
    PushInt32(a << b);
}

void ThreadedInterpreter::ExecShr() {
    int32_t b = PopInt32();
    int32_t a = PopInt32();
    PushInt32(a >> b);
}

void ThreadedInterpreter::ExecNeg() {
    int32_t value = PopInt32();
    PushInt32(-value);
}

void ThreadedInterpreter::ExecNot() {
    int32_t value = PopInt32();
    PushInt32(~value);
}

void ThreadedInterpreter::ExecCeq() {
    int32_t b = PopInt32();
    int32_t a = PopInt32();
    PushInt32(a == b ? 1 : 0);
}

void ThreadedInterpreter::ExecCgt() {
    int32_t b = PopInt32();
    int32_t a = PopInt32();
    PushInt32(a > b ? 1 : 0);
}

void ThreadedInterpreter::ExecClt() {
    int32_t b = PopInt32();
    int32_t a = PopInt32();
    PushInt32(a < b ? 1 : 0);
}

void ThreadedInterpreter::ExecBr() {
    pc_ = current_method_->instructions[pc_].operand - 1;
}

void ThreadedInterpreter::ExecBrfalse() {
    int32_t value = PopInt32();
    if (value == 0) {
        pc_ = current_method_->instructions[pc_].operand - 1;
    }
}

void ThreadedInterpreter::ExecBrtrue() {
    int32_t value = PopInt32();
    if (value != 0) {
        pc_ = current_method_->instructions[pc_].operand - 1;
    }
}

void ThreadedInterpreter::ExecCall() {}
void ThreadedInterpreter::ExecRet() { running_ = false; }
void ThreadedInterpreter::ExecBox() {}
void ThreadedInterpreter::ExecNewarr() {
    uint32_t count = PopUInt32();
    void* arr = memory_.AllocateArray(4, count);
    PushObject(arr);
}
void ThreadedInterpreter::ExecLdlen() {
    void* arr = PopObject();
    uint32_t len = *static_cast<uint32_t*>(arr);
    PushUInt32(len);
}
void ThreadedInterpreter::ExecLdelem() {
    uint32_t index = PopUInt32();
    void* arr = PopObject();
    uint8_t* base = static_cast<uint8_t*>(arr) + sizeof(uint32_t);
    uint32_t* elements = reinterpret_cast<uint32_t*>(base);
    PushInt32(static_cast<int32_t>(elements[index]));
}
void ThreadedInterpreter::ExecStelem() {
    int32_t value = PopInt32();
    uint32_t index = PopUInt32();
    void* arr = PopObject();
    uint8_t* base = static_cast<uint8_t*>(arr) + sizeof(uint32_t);
    uint32_t* elements = reinterpret_cast<uint32_t*>(base);
    elements[index] = static_cast<uint32_t>(value);
}
void ThreadedInterpreter::ExecLdfld() {
    void* obj = PopObject();
    if (obj) {
        uint32_t offset = current_method_->instructions[pc_].operand;
        void* field = static_cast<uint8_t*>(obj) + offset;
        PushObject(field);
    }
}
void ThreadedInterpreter::ExecStfld() {
    void* value = PopObject();
    void* obj = PopObject();
    if (obj) {
        uint32_t offset = current_method_->instructions[pc_].operand;
        void* field = static_cast<uint8_t*>(obj) + offset;
        std::memcpy(field, &value, sizeof(void*));
    }
}
void ThreadedInterpreter::ExecLdsfld() { PushObject(nullptr); }
void ThreadedInterpreter::ExecStsfld() { PopObject(); }
void ThreadedInterpreter::ExecNewobj() { PushObject(nullptr); }
void ThreadedInterpreter::ExecCastclass() {}
void ThreadedInterpreter::ExecIsinst() {}
void ThreadedInterpreter::ExecThrow() { running_ = false; }

void ThreadedInterpreter::PushObject(void* value) { object_stack_.push_back(value); }
void* ThreadedInterpreter::PopObject() {
    if (object_stack_.empty()) return nullptr;
    void* value = object_stack_.back();
    object_stack_.pop_back();
    return value;
}

void ThreadedInterpreter::PushInt32(int32_t value) { int32_stack_.push_back(value); }
int32_t ThreadedInterpreter::PopInt32() {
    if (int32_stack_.empty()) return 0;
    int32_t value = int32_stack_.back();
    int32_stack_.pop_back();
    return value;
}

void ThreadedInterpreter::PushUInt32(uint32_t value) { uint32_stack_.push_back(value); }
uint32_t ThreadedInterpreter::PopUInt32() {
    if (uint32_stack_.empty()) return 0;
    uint32_t value = uint32_stack_.back();
    uint32_stack_.pop_back();
    return value;
}

void ThreadedInterpreter::PushFloat(float value) { float_stack_.push_back(value); }
float ThreadedInterpreter::PopFloat() {
    if (float_stack_.empty()) return 0.0f;
    float value = float_stack_.back();
    float_stack_.pop_back();
    return value;
}

void ThreadedInterpreter::PushDouble(double value) { double_stack_.push_back(value); }
double ThreadedInterpreter::PopDouble() {
    if (double_stack_.empty()) return 0.0;
    double value = double_stack_.back();
    double_stack_.pop_back();
    return value;
}

void ThreadedInterpreter::RegisterNativeFunction(const std::string& name, NativeFunc func) {
    native_functions_[name] = std::move(func);
}

} // namespace hotc
