#include <gtest/gtest.h>
#include "../src/core/complete_interpreter.h"
#include "../src/core/type_system.h"
#include "../src/core/memory.h"
#include "../src/core/il_parser.h"
#include <cstring>

using namespace hotc;

class CompleteInterpreterTest : public ::testing::Test {
protected:
    void SetUp() override {
        types_ = std::make_unique<TypeSystem>();
        memory_ = std::make_unique<MemoryManager>();
        interp_ = std::make_unique<CompleteInterpreter>(*types_, *memory_);
    }
    std::unique_ptr<TypeSystem> types_;
    std::unique_ptr<MemoryManager> memory_;
    std::unique_ptr<CompleteInterpreter> interp_;
};

TEST_F(CompleteInterpreterTest, LdcI4) {
    MethodBody m{}; m.max_stack=1; m.max_locals=0;
    m.instructions.push_back({OpCode::Ldc_I4, 42, 0});
    m.instructions.push_back({OpCode::Ret, 0, 1});
    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 42);
}

TEST_F(CompleteInterpreterTest, LdcI4_Shortcuts) {
    for (int i = 0; i <= 8; i++) {
        MethodBody m{}; m.max_stack=1; m.max_locals=0;
        m.instructions.push_back({static_cast<OpCode>(0x16 + i), 0, 0});
        m.instructions.push_back({OpCode::Ret, 0, 1});
        interp_->Execute(m);
        EXPECT_EQ(interp_->GetReturnInt(), i);
    }
}

TEST_F(CompleteInterpreterTest, LdcI4_Neg) {
    MethodBody m{}; m.max_stack=1; m.max_locals=0;
    m.instructions.push_back({OpCode::Ldc_I4_M1, 0, 0});
    m.instructions.push_back({OpCode::Ret, 0, 1});
    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), -1);
}

TEST_F(CompleteInterpreterTest, LdcR4) {
    MethodBody m{}; m.max_stack=1; m.max_locals=0;
    float val = 3.14f; uint32_t bits;
    std::memcpy(&bits, &val, sizeof(float));
    m.instructions.push_back({OpCode::Ldc_R4, bits, 0});
    m.instructions.push_back({OpCode::Ret, 0, 1});
    interp_->Execute(m);
    EXPECT_FLOAT_EQ(interp_->GetReturnFloat(), 3.14f);
}

TEST_F(CompleteInterpreterTest, Add) {
    MethodBody m{}; m.max_stack=2; m.max_locals=0;
    m.instructions.push_back({OpCode::Ldc_I4, 3, 0});
    m.instructions.push_back({OpCode::Ldc_I4, 4, 1});
    m.instructions.push_back({OpCode::Add, 0, 2});
    m.instructions.push_back({OpCode::Ret, 0, 3});
    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 7);
}

TEST_F(CompleteInterpreterTest, Sub) {
    MethodBody m{}; m.max_stack=2; m.max_locals=0;
    m.instructions.push_back({OpCode::Ldc_I4, 10, 0});
    m.instructions.push_back({OpCode::Ldc_I4, 3, 1});
    m.instructions.push_back({OpCode::Sub, 0, 2});
    m.instructions.push_back({OpCode::Ret, 0, 3});
    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 7);
}

TEST_F(CompleteInterpreterTest, Mul) {
    MethodBody m{}; m.max_stack=2; m.max_locals=0;
    m.instructions.push_back({OpCode::Ldc_I4, 5, 0});
    m.instructions.push_back({OpCode::Ldc_I4, 6, 1});
    m.instructions.push_back({OpCode::Mul, 0, 2});
    m.instructions.push_back({OpCode::Ret, 0, 3});
    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 30);
}

TEST_F(CompleteInterpreterTest, Div) {
    MethodBody m{}; m.max_stack=2; m.max_locals=0;
    m.instructions.push_back({OpCode::Ldc_I4, 100, 0});
    m.instructions.push_back({OpCode::Ldc_I4, 7, 1});
    m.instructions.push_back({OpCode::Div, 0, 2});
    m.instructions.push_back({OpCode::Ret, 0, 3});
    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 14);
}

TEST_F(CompleteInterpreterTest, Rem) {
    MethodBody m{}; m.max_stack=2; m.max_locals=0;
    m.instructions.push_back({OpCode::Ldc_I4, 100, 0});
    m.instructions.push_back({OpCode::Ldc_I4, 7, 1});
    m.instructions.push_back({OpCode::Rem, 0, 2});
    m.instructions.push_back({OpCode::Ret, 0, 3});
    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 2);
}

TEST_F(CompleteInterpreterTest, Neg) {
    MethodBody m{}; m.max_stack=1; m.max_locals=0;
    m.instructions.push_back({OpCode::Ldc_I4, 42, 0});
    m.instructions.push_back({OpCode::Neg, 0, 1});
    m.instructions.push_back({OpCode::Ret, 0, 2});
    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), -42);
}

TEST_F(CompleteInterpreterTest, And) {
    MethodBody m{}; m.max_stack=2; m.max_locals=0;
    m.instructions.push_back({OpCode::Ldc_I4, 0xFF, 0});
    m.instructions.push_back({OpCode::Ldc_I4, 0x0F, 1});
    m.instructions.push_back({OpCode::And, 0, 2});
    m.instructions.push_back({OpCode::Ret, 0, 3});
    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 0x0F);
}

TEST_F(CompleteInterpreterTest, Or) {
    MethodBody m{}; m.max_stack=2; m.max_locals=0;
    m.instructions.push_back({OpCode::Ldc_I4, 0xF0, 0});
    m.instructions.push_back({OpCode::Ldc_I4, 0x0F, 1});
    m.instructions.push_back({OpCode::Or, 0, 2});
    m.instructions.push_back({OpCode::Ret, 0, 3});
    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 0xFF);
}

TEST_F(CompleteInterpreterTest, Xor) {
    MethodBody m{}; m.max_stack=2; m.max_locals=0;
    m.instructions.push_back({OpCode::Ldc_I4, 0xFF, 0});
    m.instructions.push_back({OpCode::Ldc_I4, 0x0F, 1});
    m.instructions.push_back({OpCode::Xor, 0, 2});
    m.instructions.push_back({OpCode::Ret, 0, 3});
    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 0xF0);
}

TEST_F(CompleteInterpreterTest, Shl) {
    MethodBody m{}; m.max_stack=2; m.max_locals=0;
    m.instructions.push_back({OpCode::Ldc_I4, 1, 0});
    m.instructions.push_back({OpCode::Ldc_I4, 4, 1});
    m.instructions.push_back({OpCode::Shl, 0, 2});
    m.instructions.push_back({OpCode::Ret, 0, 3});
    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 16);
}

TEST_F(CompleteInterpreterTest, Shr) {
    MethodBody m{}; m.max_stack=2; m.max_locals=0;
    m.instructions.push_back({OpCode::Ldc_I4, 16, 0});
    m.instructions.push_back({OpCode::Ldc_I4, 2, 1});
    m.instructions.push_back({OpCode::Shr, 0, 2});
    m.instructions.push_back({OpCode::Ret, 0, 3});
    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 4);
}

TEST_F(CompleteInterpreterTest, Ceq) {
    MethodBody m{}; m.max_stack=2; m.max_locals=0;
    m.instructions.push_back({OpCode::Ldc_I4, 5, 0});
    m.instructions.push_back({OpCode::Ldc_I4, 5, 1});
    m.instructions.push_back({OpCode::Ceq, 0, 2});
    m.instructions.push_back({OpCode::Ret, 0, 3});
    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 1);
}

TEST_F(CompleteInterpreterTest, Ceq_Noteq) {
    MethodBody m{}; m.max_stack=2; m.max_locals=0;
    m.instructions.push_back({OpCode::Ldc_I4, 5, 0});
    m.instructions.push_back({OpCode::Ldc_I4, 6, 1});
    m.instructions.push_back({OpCode::Ceq, 0, 2});
    m.instructions.push_back({OpCode::Ret, 0, 3});
    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 0);
}

TEST_F(CompleteInterpreterTest, Cgt) {
    MethodBody m{}; m.max_stack=2; m.max_locals=0;
    m.instructions.push_back({OpCode::Ldc_I4, 10, 0});
    m.instructions.push_back({OpCode::Ldc_I4, 5, 1});
    m.instructions.push_back({OpCode::Cgt, 0, 2});
    m.instructions.push_back({OpCode::Ret, 0, 3});
    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 1);
}

TEST_F(CompleteInterpreterTest, Clt) {
    MethodBody m{}; m.max_stack=2; m.max_locals=0;
    m.instructions.push_back({OpCode::Ldc_I4, 5, 0});
    m.instructions.push_back({OpCode::Ldc_I4, 10, 1});
    m.instructions.push_back({OpCode::Clt, 0, 2});
    m.instructions.push_back({OpCode::Ret, 0, 3});
    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 1);
}

TEST_F(CompleteInterpreterTest, Br) {
    MethodBody m{}; m.max_stack=1; m.max_locals=0;
    m.instructions.push_back({OpCode::Br, 2, 0});
    m.instructions.push_back({OpCode::Ldc_I4, 0, 1});
    m.instructions.push_back({OpCode::Ldc_I4, 42, 2});
    m.instructions.push_back({OpCode::Ret, 0, 3});
    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 42);
}

TEST_F(CompleteInterpreterTest, Brfalse) {
    MethodBody m{}; m.max_stack=1; m.max_locals=0;
    m.instructions.push_back({OpCode::Ldc_I4, 0, 0});
    m.instructions.push_back({OpCode::Brfalse, 3, 1});
    m.instructions.push_back({OpCode::Ldc_I4, 0, 2});
    m.instructions.push_back({OpCode::Ldc_I4, 42, 3});
    m.instructions.push_back({OpCode::Ret, 0, 4});
    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 42);
}

TEST_F(CompleteInterpreterTest, Brtrue) {
    MethodBody m{}; m.max_stack=1; m.max_locals=0;
    m.instructions.push_back({OpCode::Ldc_I4, 1, 0});
    m.instructions.push_back({OpCode::Brtrue, 3, 1});
    m.instructions.push_back({OpCode::Ldc_I4, 0, 2});
    m.instructions.push_back({OpCode::Ldc_I4, 42, 3});
    m.instructions.push_back({OpCode::Ret, 0, 4});
    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 42);
}

TEST_F(CompleteInterpreterTest, Dup) {
    MethodBody m{}; m.max_stack=2; m.max_locals=0;
    m.instructions.push_back({OpCode::Ldc_I4, 7, 0});
    m.instructions.push_back({OpCode::Dup, 0, 1});
    m.instructions.push_back({OpCode::Add, 0, 2});
    m.instructions.push_back({OpCode::Ret, 0, 3});
    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 14);
}

TEST_F(CompleteInterpreterTest, LocalVars) {
    MethodBody m{}; m.max_stack=1; m.max_locals=1;
    m.instructions.push_back({OpCode::Ldc_I4, 42, 0});
    m.instructions.push_back({OpCode::STloc_0, 0, 1});
    m.instructions.push_back({OpCode::LDloc_0, 0, 2});
    m.instructions.push_back({OpCode::Ret, 0, 3});
    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 42);
}

TEST_F(CompleteInterpreterTest, ArrayOps) {
    MethodBody m{}; m.max_stack=3; m.max_locals=1;
    m.instructions.push_back({OpCode::Ldc_I4, 5, 0});
    m.instructions.push_back({OpCode::Newarr, 0, 1});
    m.instructions.push_back({OpCode::STloc_0, 0, 2});
    m.instructions.push_back({OpCode::LDloc_0, 0, 3});
    m.instructions.push_back({OpCode::Ldc_I4, 0, 4});
    m.instructions.push_back({OpCode::Ldc_I4, 42, 5});
    m.instructions.push_back({OpCode::Stelem_I4, 0, 6});
    m.instructions.push_back({OpCode::LDloc_0, 0, 7});
    m.instructions.push_back({OpCode::Ldc_I4, 0, 8});
    m.instructions.push_back({OpCode::Ldelem_I4, 0, 9});
    m.instructions.push_back({OpCode::Ret, 0, 10});
    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 42);
}

TEST_F(CompleteInterpreterTest, Loop) {
    MethodBody m{}; m.max_stack=2; m.max_locals=2;
    m.instructions.push_back({OpCode::Ldc_I4, 0, 0});
    m.instructions.push_back({OpCode::STloc_0, 0, 1});
    m.instructions.push_back({OpCode::Ldc_I4, 0, 2});
    m.instructions.push_back({OpCode::STloc_1, 0, 3});
    m.instructions.push_back({OpCode::LDloc_1, 0, 4});
    m.instructions.push_back({OpCode::Ldc_I4, 10, 5});
    m.instructions.push_back({OpCode::Bge, 12, 6});
    m.instructions.push_back({OpCode::LDloc_0, 0, 7});
    m.instructions.push_back({OpCode::LDloc_1, 0, 8});
    m.instructions.push_back({OpCode::Add, 0, 9});
    m.instructions.push_back({OpCode::STloc_0, 0, 10});
    m.instructions.push_back({OpCode::LDloc_1, 0, 11});
    m.instructions.push_back({OpCode::Ldc_I4, 1, 12});
    m.instructions.push_back({OpCode::Add, 0, 13});
    m.instructions.push_back({OpCode::STloc_1, 0, 14});
    m.instructions.push_back({OpCode::Br, 4, 15});
    m.instructions.push_back({OpCode::LDloc_0, 0, 16});
    m.instructions.push_back({OpCode::Ret, 0, 17});
    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 45);
}

TEST_F(CompleteInterpreterTest, RegisterCompiler) {
    MethodBody m{}; m.max_stack=2; m.max_locals=0;
    m.instructions.push_back({OpCode::Ldc_I4, 3, 0});
    m.instructions.push_back({OpCode::Ldc_I4, 4, 1});
    m.instructions.push_back({OpCode::Add, 0, 2});
    m.instructions.push_back({OpCode::Ret, 0, 3});
    auto code = interp_->CompileToRegister(m);
    EXPECT_EQ(code.size(), 4u);
    EXPECT_EQ(code[0].opcode, RegOpCode::Ldc_I4);
    EXPECT_EQ(code[2].opcode, RegOpCode::Add_I4);
    EXPECT_EQ(code[3].opcode, RegOpCode::Ret);
}
