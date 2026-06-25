#include "threaded.h"
#include <cstring>
#include <stdexcept>

namespace hotc {

ThreadedInterpreter::ThreadedInterpreter(TypeSystem& types, MemoryManager& memory)
    : type_system_(types), memory_(memory) {
    InitDispatchTable();
    stack_.reserve(1024);
    locals_.reserve(256);
    int_stack_.reserve(1024);
    float_stack_.reserve(256);
    double_stack_.reserve(256);
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

    locals_.resize(method.max_locals);
    stack_.clear();
    int_stack_.clear();
    float_stack_.clear();
    double_stack_.clear();

    while (running_ && pc_ < method.instructions.size()) {
        const auto& instr = method.instructions[pc_];
        Dispatch(instr.opcode);
        pc_++;
    }
}

void ThreadedInterpreter::ExecNop() {}
void ThreadedInterpreter::ExecLdc_I4() { PushInt(current_method_->instructions[pc_].operand); }
void ThreadedInterpreter::ExecLdc_R4() {
    float value;
    std::memcpy(&value, &current_method_->instructions[pc_].operand, sizeof(float));
    PushFloat(value);
}
void ThreadedInterpreter::ExecDup() {
    void* value = Pop();
    Push(value);
    Push(value);
}
void ThreadedInterpreter::ExecPop() { Pop(); }

void ThreadedInterpreter::ExecAdd() {
    uint32_t b = PopInt();
    uint32_t a = PopInt();
    PushInt(a + b);
}

void ThreadedInterpreter::ExecSub() {
    uint32_t b = PopInt();
    uint32_t a = PopInt();
    PushInt(a - b);
}

void ThreadedInterpreter::ExecMul() {
    uint32_t b = PopInt();
    uint32_t a = PopInt();
    PushInt(a * b);
}

void ThreadedInterpreter::ExecDiv() {
    uint32_t b = PopInt();
    uint32_t a = PopInt();
    PushInt(a / b);
}

void ThreadedInterpreter::ExecRem() {
    uint32_t b = PopInt();
    uint32_t a = PopInt();
    PushInt(a % b);
}

void ThreadedInterpreter::ExecAnd() {
    uint32_t b = PopInt();
    uint32_t a = PopInt();
    PushInt(a & b);
}

void ThreadedInterpreter::ExecOr() {
    uint32_t b = PopInt();
    uint32_t a = PopInt();
    PushInt(a | b);
}

void ThreadedInterpreter::ExecXor() {
    uint32_t b = PopInt();
    uint32_t a = PopInt();
    PushInt(a ^ b);
}

void ThreadedInterpreter::ExecShl() {
    uint32_t b = PopInt();
    uint32_t a = PopInt();
    PushInt(a << b);
}

void ThreadedInterpreter::ExecShr() {
    uint32_t b = PopInt();
    int32_t a = static_cast<int32_t>(PopInt());
    PushInt(static_cast<uint32_t>(a >> b));
}

void ThreadedInterpreter::ExecNeg() {
    int32_t value = static_cast<int32_t>(PopInt());
    PushInt(static_cast<uint32_t>(-value));
}

void ThreadedInterpreter::ExecNot() {
    uint32_t value = PopInt();
    PushInt(~value);
}

void ThreadedInterpreter::ExecCeq() {
    uint32_t b = PopInt();
    uint32_t a = PopInt();
    PushInt(a == b ? 1 : 0);
}

void ThreadedInterpreter::ExecCgt() {
    uint32_t b = PopInt();
    uint32_t a = PopInt();
    PushInt(a > b ? 1 : 0);
}

void ThreadedInterpreter::ExecClt() {
    uint32_t b = PopInt();
    uint32_t a = PopInt();
    PushInt(a < b ? 1 : 0);
}

void ThreadedInterpreter::ExecBr() {
    pc_ = current_method_->instructions[pc_].operand - 1;
}

void ThreadedInterpreter::ExecBrfalse() {
    uint32_t value = PopInt();
    if (value == 0) {
        pc_ = current_method_->instructions[pc_].operand - 1;
    }
}

void ThreadedInterpreter::ExecBrtrue() {
    uint32_t value = PopInt();
    if (value != 0) {
        pc_ = current_method_->instructions[pc_].operand - 1;
    }
}

void ThreadedInterpreter::ExecCall() {}
void ThreadedInterpreter::ExecRet() { running_ = false; }
void ThreadedInterpreter::ExecBox() {}
void ThreadedInterpreter::ExecNewarr() {}
void ThreadedInterpreter::ExecLdlen() {}
void ThreadedInterpreter::ExecLdfld() {}
void ThreadedInterpreter::ExecStfld() {}
void ThreadedInterpreter::ExecLdsfld() {}
void ThreadedInterpreter::ExecStsfld() {}
void ThreadedInterpreter::ExecNewobj() {}
void ThreadedInterpreter::ExecCastclass() {}
void ThreadedInterpreter::ExecIsinst() {}
void ThreadedInterpreter::ExecThrow() { running_ = false; }

void ThreadedInterpreter::ExecLdarg() {}
void ThreadedInterpreter::ExecStarg() {}
void ThreadedInterpreter::ExecLdloc() {}
void ThreadedInterpreter::ExecStloc() {}
void ThreadedInterpreter::ExecLdelem() {}
void ThreadedInterpreter::ExecStelem() {}

void ThreadedInterpreter::Push(void* value) { stack_.push_back(value); }
void* ThreadedInterpreter::Pop() {
    void* value = stack_.back();
    stack_.pop_back();
    return value;
}

void ThreadedInterpreter::PushInt(uint32_t value) { int_stack_.push_back(value); }
uint32_t ThreadedInterpreter::PopInt() {
    uint32_t value = int_stack_.back();
    int_stack_.pop_back();
    return value;
}

void ThreadedInterpreter::PushFloat(float value) { float_stack_.push_back(value); }
float ThreadedInterpreter::PopFloat() {
    float value = float_stack_.back();
    float_stack_.pop_back();
    return value;
}

void ThreadedInterpreter::RegisterNativeFunction(const std::string& name, NativeFunc func) {
    native_functions_[name] = std::move(func);
}

} // namespace hotc
