#include "complete_interpreter.h"
#include <cstring>
#include <cmath>

namespace hotc {

// GenericKey comparison
bool CompleteInterpreter::GenericKey::operator==(const GenericKey& o) const {
    return method_index == o.method_index && type_args == o.type_args;
}

size_t CompleteInterpreter::GenericKeyHash::operator()(const GenericKey& k) const {
    size_t h = std::hash<uint32_t>()(k.method_index);
    for (auto t : k.type_args) {
        h ^= std::hash<uint32_t>()(t) + 0x9e3779b9 + (h << 6) + (h >> 2);
    }
    return h;
}

CompleteInterpreter::CompleteInterpreter(TypeSystem& types, MemoryManager& memory)
    : types_(types), memory_(memory) {
    i32_stack_.reserve(4096);
    u32_stack_.reserve(1024);
    f32_stack_.reserve(1024);
    f64_stack_.reserve(512);
    obj_stack_.reserve(1024);
    locals_.resize(256);

    // Allocate call frames
    for (uint32_t i = 0; i < 64; i++) {
        frames_[i].prev = (i > 0) ? &frames_[i - 1] : nullptr;
    }
}

// ========== Stack Operations ==========

void CompleteInterpreter::PushI32(int32_t v) { i32_stack_.push_back(v); }
int32_t CompleteInterpreter::PopI32() {
    int32_t v = i32_stack_.back();
    i32_stack_.pop_back();
    return v;
}

void CompleteInterpreter::PushU32(uint32_t v) { u32_stack_.push_back(v); }
uint32_t CompleteInterpreter::PopU32() {
    uint32_t v = u32_stack_.back();
    u32_stack_.pop_back();
    return v;
}

void CompleteInterpreter::PushF32(float v) { f32_stack_.push_back(v); }
float CompleteInterpreter::PopF32() {
    float v = f32_stack_.back();
    f32_stack_.pop_back();
    return v;
}

void CompleteInterpreter::PushF64(double v) { f64_stack_.push_back(v); }
double CompleteInterpreter::PopF64() {
    double v = f64_stack_.back();
    f64_stack_.pop_back();
    return v;
}

void CompleteInterpreter::PushObj(void* v) { obj_stack_.push_back(v); }
void* CompleteInterpreter::PopObj() {
    void* v = obj_stack_.back();
    obj_stack_.pop_back();
    return v;
}

// ========== Call Frame Management ==========

void CompleteInterpreter::PushFrame(const MethodBody* method, uint32_t pc,
                                     uint32_t stack_base, void* ret_addr) {
    if (frame_depth_ >= 64) return;

    CallFrame& f = frames_[frame_depth_++];
    f.method = current_method_;
    f.pc = pc_;
    f.stack_base = stack_base;
    f.return_addr = ret_addr;
    f.prev = frame_;
    frame_ = &f;
}

void CompleteInterpreter::PopFrame() {
    if (frame_depth_ == 0) return;
    CallFrame* f = frame_;
    frame_ = f->prev;
    frame_depth_--;

    current_method_ = f->method;
    pc_ = f->pc;
}

void CompleteInterpreter::CallMethod(uint32_t method_index, void** args, uint32_t arg_count) {
    // Try native first
    for (auto& [name, func] : native_funcs_) {
        // Simple name-based lookup (in real impl, would use method index)
        (void)name;
        (void)func;
    }

    // Look up method body
    auto it = method_bodies_.find(method_index);
    if (it == method_bodies_.end()) return;

    const MethodBody* body = it->second;
    if (!body) return;

    // Save current state
    uint32_t saved_stack_base = static_cast<uint32_t>(i32_stack_.size());
    uint32_t saved_obj_base = static_cast<uint32_t>(obj_stack_.size());

    // Push call frame
    PushFrame(current_method_, pc_, saved_stack_base, nullptr);

    // Setup new method
    current_method_ = body;
    pc_ = 0;

    // Setup locals
    uint32_t total_locals = body->max_locals + arg_count;
    if (total_locals > locals_.size()) {
        locals_.resize(total_locals);
    }

    // Copy arguments to locals
    for (uint32_t i = 0; i < arg_count; i++) {
        locals_[i] = args ? args[i] : nullptr;
    }

    // Execute
    ExecuteLoop();

    // Restore state
    PopFrame();

    // Stack cleanup - pop any values pushed during call
    while (static_cast<uint32_t>(obj_stack_.size()) > saved_obj_base) {
        obj_stack_.pop_back();
    }

    // The return value should be on the stack
}

// ========== Main Execution Loop ==========

void CompleteInterpreter::Execute(const MethodBody& method, void** args, uint32_t arg_count) {
    current_method_ = &method;
    pc_ = 0;
    running_ = true;
    frame_depth_ = 0;
    frame_ = nullptr;

    // Reset stacks
    i32_stack_.clear();
    u32_stack_.clear();
    f32_stack_.clear();
    f64_stack_.clear();
    obj_stack_.clear();

    // Setup locals
    uint32_t total_locals = method.max_locals + arg_count;
    locals_.resize(total_locals, nullptr);

    // Copy arguments
    if (args && arg_count > 0) {
        for (uint32_t i = 0; i < arg_count; i++) {
            locals_[i] = args[i];
        }
    }

    ExecuteLoop();
}

void CompleteInterpreter::ExecuteLoop() {
    while (running_ && pc_ < static_cast<uint32_t>(current_method_->instructions.size())) {
        const Instruction& instr = current_method_->instructions[pc_];
        pc_++;

        switch (instr.opcode) {
            // ===== Nop =====
            case OpCode::Nop:
                break;

            // ===== Load Constants =====
            case OpCode::Ldc_I4_0: PushI32(0); break;
            case OpCode::Ldc_I4_1: PushI32(1); break;
            case OpCode::Ldc_I4_2: PushI32(2); break;
            case OpCode::Ldc_I4_3: PushI32(3); break;
            case OpCode::Ldc_I4_4: PushI32(4); break;
            case OpCode::Ldc_I4_5: PushI32(5); break;
            case OpCode::Ldc_I4_6: PushI32(6); break;
            case OpCode::Ldc_I4_7: PushI32(7); break;
            case OpCode::Ldc_I4_8: PushI32(8); break;
            case OpCode::Ldc_I4_M1: PushI32(-1); break;
            case OpCode::Ldc_I4:
            case OpCode::Ldc_I4_S:
                PushI32(static_cast<int32_t>(instr.operand));
                break;

            case OpCode::Ldc_R4: {
                float val;
                std::memcpy(&val, &instr.operand, sizeof(float));
                PushF32(val);
                break;
            }

            case OpCode::Ldc_R8: {
                uint32_t low = instr.operand;
                uint32_t high = (pc_ < static_cast<uint32_t>(current_method_->instructions.size()))
                    ? current_method_->instructions[pc_].operand : 0;
                uint64_t raw = (static_cast<uint64_t>(high) << 32) | low;
                double val;
                std::memcpy(&val, &raw, sizeof(double));
                PushF64(val);
                pc_++; // skip high word
                break;
            }

            // ===== Load/Store Locals =====
            case OpCode::LDloc_0: PushObj(locals_[0]); break;
            case OpCode::LDloc_1: PushObj(locals_[1]); break;
            case OpCode::LDloc_2: PushObj(locals_[2]); break;
            case OpCode::LDloc_3: PushObj(locals_[3]); break;
            case OpCode::LDloc_S:
            case OpCode::LDloc:
                PushObj(locals_[instr.operand]);
                break;

            case OpCode::STloc_0: locals_[0] = PopObj(); break;
            case OpCode::STloc_1: locals_[1] = PopObj(); break;
            case OpCode::STloc_2: locals_[2] = PopObj(); break;
            case OpCode::STloc_3: locals_[3] = PopObj(); break;
            case OpCode::STloc_S:
            case OpCode::STloc:
                locals_[instr.operand] = PopObj();
                break;

            // ===== Load/Store Arguments =====
            case OpCode::LDarg_0: PushObj(locals_[0]); break;
            case OpCode::LDarg_1: PushObj(locals_[1]); break;
            case OpCode::LDarg_2: PushObj(locals_[2]); break;
            case OpCode::LDarg_3: PushObj(locals_[3]); break;
            case OpCode::LDarg_S:
            case OpCode::LDarg:
                PushObj(locals_[instr.operand]);
                break;

            case OpCode::STarg:
                locals_[instr.operand] = PopObj();
                break;

            // ===== Stack Manipulation =====
            case OpCode::Dup: {
                void* v = PopObj();
                PushObj(v);
                PushObj(v);
                break;
            }
            case OpCode::Pop:
                PopObj();
                break;

            // ===== Integer Arithmetic =====
            case OpCode::Add: {
                int32_t b = PopI32();
                int32_t a = PopI32();
                PushI32(a + b);
                break;
            }
            case OpCode::Sub: {
                int32_t b = PopI32();
                int32_t a = PopI32();
                PushI32(a - b);
                break;
            }
            case OpCode::Mul: {
                int32_t b = PopI32();
                int32_t a = PopI32();
                PushI32(a * b);
                break;
            }
            case OpCode::Div: {
                int32_t b = PopI32();
                int32_t a = PopI32();
                PushI32(a / b);
                break;
            }
            case OpCode::Div_Un: {
                uint32_t b = PopU32();
                uint32_t a = PopU32();
                PushU32(a / b);
                break;
            }
            case OpCode::Rem: {
                int32_t b = PopI32();
                int32_t a = PopI32();
                PushI32(a % b);
                break;
            }
            case OpCode::Rem_Un: {
                uint32_t b = PopU32();
                uint32_t a = PopU32();
                PushU32(a % b);
                break;
            }

            // ===== Bitwise Operations =====
            case OpCode::And: {
                int32_t b = PopI32();
                int32_t a = PopI32();
                PushI32(a & b);
                break;
            }
            case OpCode::Or: {
                int32_t b = PopI32();
                int32_t a = PopI32();
                PushI32(a | b);
                break;
            }
            case OpCode::Xor: {
                int32_t b = PopI32();
                int32_t a = PopI32();
                PushI32(a ^ b);
                break;
            }
            case OpCode::Shl: {
                int32_t b = PopI32();
                int32_t a = PopI32();
                PushI32(a << b);
                break;
            }
            case OpCode::Shr: {
                int32_t b = PopI32();
                int32_t a = PopI32();
                PushI32(a >> b);
                break;
            }
            case OpCode::Shr_Un: {
                uint32_t b = PopU32();
                uint32_t a = PopU32();
                PushU32(a >> b);
                break;
            }

            // ===== Unary Operations =====
            case OpCode::Neg: {
                int32_t v = PopI32();
                PushI32(-v);
                break;
            }
            case OpCode::Not: {
                int32_t v = PopI32();
                PushI32(~v);
                break;
            }

            // ===== Integer Comparisons =====
            case OpCode::Ceq: {
                int32_t b = PopI32();
                int32_t a = PopI32();
                PushI32(a == b ? 1 : 0);
                break;
            }
            case OpCode::Cgt: {
                int32_t b = PopI32();
                int32_t a = PopI32();
                PushI32(a > b ? 1 : 0);
                break;
            }
            case OpCode::Clt: {
                int32_t b = PopI32();
                int32_t a = PopI32();
                PushI32(a < b ? 1 : 0);
                break;
            }

            // ===== Branch Instructions =====
            case OpCode::Br:
            case OpCode::Br_S:
                pc_ = instr.operand;
                break;

            case OpCode::Brfalse:
            case OpCode::Brfalse_S: {
                int32_t v = PopI32();
                if (v == 0) pc_ = instr.operand;
                break;
            }
            case OpCode::Brtrue:
            case OpCode::Brtrue_S: {
                int32_t v = PopI32();
                if (v != 0) pc_ = instr.operand;
                break;
            }

            // ===== Conditional Branches =====
            case OpCode::Beq: case OpCode::Beq_S: {
                int32_t b = PopI32(); int32_t a = PopI32();
                if (a == b) pc_ = instr.operand;
                break;
            }
            case OpCode::Bge: case OpCode::Bge_S: {
                int32_t b = PopI32(); int32_t a = PopI32();
                if (a >= b) pc_ = instr.operand;
                break;
            }
            case OpCode::Bgt: case OpCode::Bgt_S: {
                int32_t b = PopI32(); int32_t a = PopI32();
                if (a > b) pc_ = instr.operand;
                break;
            }
            case OpCode::Ble: case OpCode::Ble_S: {
                int32_t b = PopI32(); int32_t a = PopI32();
                if (a <= b) pc_ = instr.operand;
                break;
            }
            case OpCode::Blt: case OpCode::Blt_S: {
                int32_t b = PopI32(); int32_t a = PopI32();
                if (a < b) pc_ = instr.operand;
                break;
            }
            case OpCode::Bne_Un: case OpCode::Bne_Un_S: {
                uint32_t b = PopU32(); uint32_t a = PopU32();
                if (a != b) pc_ = instr.operand;
                break;
            }
            case OpCode::Bge_Un: case OpCode::Bge_Un_S: {
                uint32_t b = PopU32(); uint32_t a = PopU32();
                if (a >= b) pc_ = instr.operand;
                break;
            }
            case OpCode::Bgt_Un: case OpCode::Bgt_Un_S: {
                uint32_t b = PopU32(); uint32_t a = PopU32();
                if (a > b) pc_ = instr.operand;
                break;
            }
            case OpCode::Ble_Un: case OpCode::Ble_Un_S: {
                uint32_t b = PopU32(); uint32_t a = PopU32();
                if (a <= b) pc_ = instr.operand;
                break;
            }
            case OpCode::Blt_Un: case OpCode::Blt_Un_S: {
                uint32_t b = PopU32(); uint32_t a = PopU32();
                if (a < b) pc_ = instr.operand;
                break;
            }

            // ===== Method Call =====
            case OpCode::Call: {
                uint32_t method_idx = instr.operand;

                // Check native function registry first
                bool found_native = false;
                for (auto& [name, func] : native_funcs_) {
                    // In real impl, match by method_index
                    // For now, just call the first native we find with matching index
                }

                if (!found_native) {
                    // Get method body and call it
                    CallMethod(method_idx, nullptr, 0);
                }
                break;
            }

            case OpCode::Calli: {
                // Indirect call - not commonly used
                break;
            }

            // ===== Return =====
            case OpCode::Ret:
                running_ = false;
                break;

            // ===== Object Operations =====
            case OpCode::Box: {
                int32_t v = PopI32();
                void* boxed = memory_.AllocateObject(sizeof(int32_t));
                std::memcpy(boxed, &v, sizeof(int32_t));
                PushObj(boxed);
                break;
            }

            case OpCode::Newarr: {
                uint32_t count = PopU32();
                void* arr = memory_.AllocateArray(4, count);
                PushObj(arr);
                break;
            }

            case OpCode::Ldlen: {
                void* arr = PopObj();
                uint32_t len = *static_cast<uint32_t*>(arr);
                PushI32(static_cast<int32_t>(len));
                break;
            }

            case OpCode::Ldelem_I4: {
                uint32_t idx = PopU32();
                void* arr = PopObj();
                uint32_t* elems = reinterpret_cast<uint32_t*>(
                    static_cast<uint8_t*>(arr) + sizeof(uint32_t));
                PushI32(static_cast<int32_t>(elems[idx]));
                break;
            }

            case OpCode::Stelem_I4: {
                int32_t val = PopI32();
                uint32_t idx = PopU32();
                void* arr = PopObj();
                uint32_t* elems = reinterpret_cast<uint32_t*>(
                    static_cast<uint8_t*>(arr) + sizeof(uint32_t));
                elems[idx] = static_cast<uint32_t>(val);
                break;
            }

            case OpCode::Ldelem_R4: {
                uint32_t idx = PopU32();
                void* arr = PopObj();
                float* elems = reinterpret_cast<float*>(
                    static_cast<uint8_t*>(arr) + sizeof(uint32_t));
                PushF32(elems[idx]);
                break;
            }

            case OpCode::Stelem_R4: {
                float val = PopF32();
                uint32_t idx = PopU32();
                void* arr = PopObj();
                float* elems = reinterpret_cast<float*>(
                    static_cast<uint8_t*>(arr) + sizeof(uint32_t));
                elems[idx] = val;
                break;
            }

            case OpCode::Ldelem_Ref: {
                uint32_t idx = PopU32();
                void* arr = PopObj();
                void** elems = reinterpret_cast<void**>(
                    static_cast<uint8_t*>(arr) + sizeof(uint32_t));
                PushObj(elems[idx]);
                break;
            }

            case OpCode::Stelem_Ref: {
                void* val = PopObj();
                uint32_t idx = PopU32();
                void* arr = PopObj();
                void** elems = reinterpret_cast<void**>(
                    static_cast<uint8_t*>(arr) + sizeof(uint32_t));
                elems[idx] = val;
                break;
            }

            // ===== Field Access =====
            case OpCode::Ldfld: {
                uint32_t offset = instr.operand;
                void* obj = PopObj();
                if (obj) {
                    void* field = static_cast<uint8_t*>(obj) + offset;
                    PushObj(field);
                }
                break;
            }

            case OpCode::Stfld: {
                uint32_t offset = instr.operand;
                void* val = PopObj();
                void* obj = PopObj();
                if (obj) {
                    void* field = static_cast<uint8_t*>(obj) + offset;
                    std::memcpy(field, &val, sizeof(void*));
                }
                break;
            }

            case OpCode::Ldsfld: {
                PushObj(nullptr); // Static field stub
                break;
            }

            case OpCode::Stsfld: {
                PopObj(); // Static field stub
                break;
            }

            // ===== Object Creation =====
            case OpCode::Newobj: {
                // Allocate object and push it
                void* obj = memory_.AllocateObject(32);
                PushObj(obj);
                break;
            }

            case OpCode::Castclass:
            case OpCode::Isinst: {
                // Type check - pass through for now
                break;
            }

            case OpCode::Ldnull:
                PushObj(nullptr);
                break;

            case OpCode::Ldstr: {
                // String literal - push null for now
                PushObj(nullptr);
                break;
            }

            // ===== Exception Handling =====
            case OpCode::Throw: {
                void* exc = PopObj();
                running_ = false;
                break;
            }

            case OpCode::Rethrow:
                running_ = false;
                break;

            default:
                break;
        }
    }
}

// ========== Register Compiler ==========

std::vector<RegInstruction> CompleteInterpreter::CompileToRegister(const MethodBody& method) {
    std::vector<RegInstruction> code;
    code.reserve(method.instructions.size());

    for (const auto& instr : method.instructions) {
        RegInstruction ri;
        ri.dst = 0;
        ri.src1 = 0;
        ri.src2 = 0;
        ri.operand = instr.operand;

        switch (instr.opcode) {
            case OpCode::Nop:
                ri.opcode = RegOpCode::Nop;
                break;

            case OpCode::Ldc_I4:
            case OpCode::Ldc_I4_S:
                ri.opcode = RegOpCode::Ldc_I4;
                ri.operand = instr.operand;
                break;

            case OpCode::Ldc_I4_0: ri.opcode = RegOpCode::Ldc_I4; ri.operand = 0; break;
            case OpCode::Ldc_I4_1: ri.opcode = RegOpCode::Ldc_I4; ri.operand = 1; break;
            case OpCode::Ldc_I4_2: ri.opcode = RegOpCode::Ldc_I4; ri.operand = 2; break;
            case OpCode::Ldc_I4_3: ri.opcode = RegOpCode::Ldc_I4; ri.operand = 3; break;
            case OpCode::Ldc_I4_4: ri.opcode = RegOpCode::Ldc_I4; ri.operand = 4; break;
            case OpCode::Ldc_I4_5: ri.opcode = RegOpCode::Ldc_I4; ri.operand = 5; break;
            case OpCode::Ldc_I4_6: ri.opcode = RegOpCode::Ldc_I4; ri.operand = 6; break;
            case OpCode::Ldc_I4_7: ri.opcode = RegOpCode::Ldc_I4; ri.operand = 7; break;
            case OpCode::Ldc_I4_8: ri.opcode = RegOpCode::Ldc_I4; ri.operand = 8; break;
            case OpCode::Ldc_I4_M1: ri.opcode = RegOpCode::Ldc_I4; ri.operand = static_cast<uint32_t>(-1); break;

            case OpCode::Ldc_R4:
                ri.opcode = RegOpCode::Ldc_R4;
                break;

            case OpCode::Ldc_R8:
                ri.opcode = RegOpCode::Ldc_R8;
                break;

            case OpCode::LDloc_0: ri.opcode = RegOpCode::Ldloc; ri.operand = 0; break;
            case OpCode::LDloc_1: ri.opcode = RegOpCode::Ldloc; ri.operand = 1; break;
            case OpCode::LDloc_2: ri.opcode = RegOpCode::Ldloc; ri.operand = 2; break;
            case OpCode::LDloc_3: ri.opcode = RegOpCode::Ldloc; ri.operand = 3; break;
            case OpCode::LDloc_S:
            case OpCode::LDloc:
                ri.opcode = RegOpCode::Ldloc;
                break;

            case OpCode::STloc_0: ri.opcode = RegOpCode::Stloc; ri.operand = 0; break;
            case OpCode::STloc_1: ri.opcode = RegOpCode::Stloc; ri.operand = 1; break;
            case OpCode::STloc_2: ri.opcode = RegOpCode::Stloc; ri.operand = 2; break;
            case OpCode::STloc_3: ri.opcode = RegOpCode::Stloc; ri.operand = 3; break;
            case OpCode::STloc_S:
            case OpCode::STloc:
                ri.opcode = RegOpCode::Stloc;
                break;

            case OpCode::LDarg_0: ri.opcode = RegOpCode::Ldarg; ri.operand = 0; break;
            case OpCode::LDarg_1: ri.opcode = RegOpCode::Ldarg; ri.operand = 1; break;
            case OpCode::LDarg_2: ri.opcode = RegOpCode::Ldarg; ri.operand = 2; break;
            case OpCode::LDarg_3: ri.opcode = RegOpCode::Ldarg; ri.operand = 3; break;
            case OpCode::LDarg_S:
            case OpCode::LDarg:
                ri.opcode = RegOpCode::Ldarg;
                break;

            case OpCode::STarg:
                ri.opcode = RegOpCode::Starg;
                break;

            case OpCode::Add: ri.opcode = RegOpCode::Add_I4; break;
            case OpCode::Sub: ri.opcode = RegOpCode::Sub_I4; break;
            case OpCode::Mul: ri.opcode = RegOpCode::Mul_I4; break;
            case OpCode::Div: ri.opcode = RegOpCode::Div_I4; break;
            case OpCode::Rem: ri.opcode = RegOpCode::Rem_I4; break;
            case OpCode::And: ri.opcode = RegOpCode::And_I4; break;
            case OpCode::Or: ri.opcode = RegOpCode::Or_I4; break;
            case OpCode::Xor: ri.opcode = RegOpCode::Xor_I4; break;
            case OpCode::Shl: ri.opcode = RegOpCode::Shl_I4; break;
            case OpCode::Shr: ri.opcode = RegOpCode::Shr_I4; break;
            case OpCode::Neg: ri.opcode = RegOpCode::Neg_I4; break;
            case OpCode::Not: ri.opcode = RegOpCode::Not_I4; break;
            case OpCode::Ceq: ri.opcode = RegOpCode::Ceq_I4; break;
            case OpCode::Cgt: ri.opcode = RegOpCode::Cgt_I4; break;
            case OpCode::Clt: ri.opcode = RegOpCode::Clt_I4; break;

            case OpCode::Br:
            case OpCode::Br_S:
                ri.opcode = RegOpCode::Br;
                break;

            case OpCode::Brfalse:
            case OpCode::Brfalse_S:
                ri.opcode = RegOpCode::Brfalse;
                break;

            case OpCode::Brtrue:
            case OpCode::Brtrue_S:
                ri.opcode = RegOpCode::Brtrue;
                break;

            case OpCode::Beq: case OpCode::Beq_S: ri.opcode = RegOpCode::Beq; break;
            case OpCode::Bge: case OpCode::Bge_S: ri.opcode = RegOpCode::Bge; break;
            case OpCode::Bgt: case OpCode::Bgt_S: ri.opcode = RegOpCode::Bgt; break;
            case OpCode::Ble: case OpCode::Ble_S: ri.opcode = RegOpCode::Ble; break;
            case OpCode::Blt: case OpCode::Blt_S: ri.opcode = RegOpCode::Blt; break;

            case OpCode::Call: ri.opcode = RegOpCode::Call; break;
            case OpCode::Ret: ri.opcode = RegOpCode::Ret; break;

            case OpCode::Box: ri.opcode = RegOpCode::Box; break;
            case OpCode::Newarr: ri.opcode = RegOpCode::Newarr; break;
            case OpCode::Ldlen: ri.opcode = RegOpCode::Ldlen; break;
            case OpCode::Ldelem_I4: ri.opcode = RegOpCode::Ldelem_I4; break;
            case OpCode::Stelem_I4: ri.opcode = RegOpCode::Stelem_I4; break;
            case OpCode::Ldfld: ri.opcode = RegOpCode::Ldfld; break;
            case OpCode::Stfld: ri.opcode = RegOpCode::Stfld; break;
            case OpCode::Ldsfld: ri.opcode = RegOpCode::Ldsfld; break;
            case OpCode::Stsfld: ri.opcode = RegOpCode::Stsfld; break;
            case OpCode::Newobj: ri.opcode = RegOpCode::Newobj; break;
            case OpCode::Castclass: ri.opcode = RegOpCode::Castclass; break;
            case OpCode::Isinst: ri.opcode = RegOpCode::Isinst; break;
            case OpCode::Throw: ri.opcode = RegOpCode::Throw; break;
            case OpCode::Ldnull: ri.opcode = RegOpCode::Ldnull; break;
            case OpCode::Dup: ri.opcode = RegOpCode::Dup; break;
            case OpCode::Pop: ri.opcode = RegOpCode::Pop; break;
            case OpCode::Ldstr: ri.opcode = RegOpCode::Ldstr; break;

            default:
                ri.opcode = RegOpCode::Nop;
                break;
        }

        code.push_back(ri);
    }

    return code;
}

// ========== Type Operations ==========

bool CompleteInterpreter::IsInstanceOf(uint32_t obj_type, uint32_t target_type) const {
    if (obj_type == target_type) return true;
    return IsSubclass(obj_type, target_type);
}

bool CompleteInterpreter::IsSubclass(uint32_t child, uint32_t parent) const {
    if (child == parent) return true;
    uint32_t current = child;
    while (current != 0) {
        auto it = parent_type_.find(current);
        if (it == parent_type_.end()) return false;
        if (it->second == parent) return true;
        current = it->second;
    }
    return false;
}

// ========== Method Body Registry ==========

const MethodBody* CompleteInterpreter::GetMethodBody(uint32_t method_index) const {
    auto it = method_bodies_.find(method_index);
    return (it != method_bodies_.end()) ? it->second : nullptr;
}

void CompleteInterpreter::SetMethodBody(uint32_t method_index, const MethodBody* body) {
    method_bodies_[method_index] = body;
}

void CompleteInterpreter::RegisterNative(const std::string& name, NativeFunc func) {
    native_funcs_[name] = func;
}

} // namespace hotc
