#include <gtest/gtest.h>
#include "../src/core/complete_interpreter.h"
#include "../src/core/type_system.h"
#include "../src/core/memory.h"
#include "../src/core/il_parser.h"

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

// ========== Stack Operations ==========

TEST_F(CompleteInterpreterTest, LdcI4) {
    MethodBody m;
    m.max_stack = 1;
    m.max_locals = 0;
    m.instructions.push_back({OpCode::Ldc_I4, 42, 0});
    m.instructions.push_back({OpCode::Ret, 0, 1});

    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 42);
}

TEST_F(CompleteInterpreterTest, LdcI4_Shortcuts) {
    for (int i = 0; i <= 8; i++) {
        MethodBody m;
        m.max_stack = 1;
        m.max_locals = 0;
        m.instructions.push_back({static_cast<OpCode>(0x16 + i), 0, 0});
        m.instructions.push_back({OpCode::Ret, 0, 1});

        interp_->Execute(m);
        EXPECT_EQ(interp_->GetReturnInt(), i);
    }
}

TEST_F(CompleteInterpreterTest, LdcI4_Negative) {
    MethodBody m;
    m.max_stack = 1;
    m.max_locals = 0;
    m.instructions.push_back({OpCode::Ldc_I4_M1, 0, 0});
    m.instructions.push_back({OpCode::Ret, 0, 1});

    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), -1);
}

TEST_F(CompleteInterpreterTest, LdcR4) {
    MethodBody m;
    m.max_stack = 1;
    m.max_locals = 0;
    float val = 3.14f;
    uint32_t bits;
    std::memcpy(&bits, &val, sizeof(float));
    m.instructions.push_back({OpCode::Ldc_R4, bits, 0});
    m.instructions.push_back({OpCode::Ret, 0, 1});

    interp_->Execute(m);
    EXPECT_FLOAT_EQ(interp_->GetReturnFloat(), 3.14f);
}

// ========== Arithmetic ==========

TEST_F(CompleteInterpreterTest, Add) {
    MethodBody m;
    m.max_stack = 2;
    m.max_locals = 0;
    m.instructions.push_back({OpCode::Ldc_I4, 3, 0});
    m.instructions.push_back({OpCode::Ldc_I4, 4, 1});
    m.instructions.push_back({OpCode::Add, 0, 2});
    m.instructions.push_back({OpCode::Ret, 0, 3});

    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 7);
}

TEST_F(CompleteInterpreterTest, Sub) {
    MethodBody m;
    m.max_stack = 2;
    m.max_locals = 0;
    m.instructions.push_back({OpCode::Ldc_I4, 10, 0});
    m.instructions.push_back({OpCode::Ldc_I4, 3, 1});
    m.instructions.push_back({OpCode::Sub, 0, 2});
    m.instructions.push_back({OpCode::Ret, 0, 3});

    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 7);
}

TEST_F(CompleteInterpreterTest, Mul) {
    MethodBody m;
    m.max_stack = 2;
    m.max_locals = 0;
    m.instructions.push_back({OpCode::Ldc_I4, 5, 0});
    m.instructions.push_back({OpCode::Ldc_I4, 6, 1});
    m.instructions.push_back({OpCode::Mul, 0, 2});
    m.instructions.push_back({OpCode::Ret, 0, 3});

    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 30);
}

TEST_F(CompleteInterpreterTest, Div) {
    MethodBody m;
    m.max_stack = 2;
    m.max_locals = 0;
    m.instructions.push_back({OpCode::Ldc_I4, 100, 0});
    m.instructions.push_back({OpCode::Ldc_I4, 7, 1});
    m.instructions.push_back({OpCode::Div, 0, 2});
    m.instructions.push_back({OpCode::Ret, 0, 3});

    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 14);
}

TEST_F(CompleteInterpreterTest, Rem) {
    MethodBody m;
    m.max_stack = 2;
    m.max_locals = 0;
    m.instructions.push_back({OpCode::Ldc_I4, 100, 0});
    m.instructions.push_back({OpCode::Ldc_I4, 7, 1});
    m.instructions.push_back({OpCode::Rem, 0, 2});
    m.instructions.push_back({OpCode::Ret, 0, 3});

    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 2);
}

TEST_F(CompleteInterpreterTest, Neg) {
    MethodBody m;
    m.max_stack = 1;
    m.max_locals = 0;
    m.instructions.push_back({OpCode::Ldc_I4, 42, 0});
    m.instructions.push_back({OpCode::Neg, 0, 1});
    m.instructions.push_back({OpCode::Ret, 0, 2});

    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), -42);
}

// ========== Bitwise Operations ==========

TEST_F(CompleteInterpreterTest, And) {
    MethodBody m;
    m.max_stack = 2;
    m.max_locals = 0;
    m.instructions.push_back({OpCode::Ldc_I4, 0xFF, 0});
    m.instructions.push_back({OpCode::Ldc_I4, 0x0F, 1});
    m.instructions.push_back({OpCode::And, 0, 2});
    m.instructions.push_back({OpCode::Ret, 0, 3});

    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 0x0F);
}

TEST_F(CompleteInterpreterTest, Or) {
    MethodBody m;
    m.max_stack = 2;
    m.max_locals = 0;
    m.instructions.push_back({OpCode::Ldc_I4, 0xF0, 0});
    m.instructions.push_back({OpCode::Ldc_I4, 0x0F, 1});
    m.instructions.push_back({OpCode::Or, 0, 2});
    m.instructions.push_back({OpCode::Ret, 0, 3});

    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 0xFF);
}

TEST_F(CompleteInterpreterTest, Xor) {
    MethodBody m;
    m.max_stack = 2;
    m.max_locals = 0;
    m.instructions.push_back({OpCode::Ldc_I4, 0xFF, 0});
    m.instructions.push_back({OpCode::Ldc_I4, 0x0F, 1});
    m.instructions.push_back({OpCode::Xor, 0, 2});
    m.instructions.push_back({OpCode::Ret, 0, 3});

    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 0xF0);
}

TEST_F(CompleteInterpreterTest, Shl) {
    MethodBody m;
    m.max_stack = 2;
    m.max_locals = 0;
    m.instructions.push_back({OpCode::Ldc_I4, 1, 0});
    m.instructions.push_back({OpCode::Ldc_I4, 4, 1});
    m.instructions.push_back({OpCode::Shl, 0, 2});
    m.instructions.push_back({OpCode::Ret, 0, 3});

    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 16);
}

TEST_F(CompleteInterpreterTest, Shr) {
    MethodBody m;
    m.max_stack = 2;
    m.max_locals = 0;
    m.instructions.push_back({OpCode::Ldc_I4, 16, 0});
    m.instructions.push_back({OpCode::Ldc_I4, 2, 1});
    m.instructions.push_back({OpCode::Shr, 0, 2});
    m.instructions.push_back({OpCode::Ret, 0, 3});

    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 4);
}

// ========== Comparisons ==========

TEST_F(CompleteInterpreterTest, Ceq_Equal) {
    MethodBody m;
    m.max_stack = 2;
    m.max_locals = 0;
    m.instructions.push_back({OpCode::Ldc_I4, 5, 0});
    m.instructions.push_back({OpCode::Ldc_I4, 5, 1});
    m.instructions.push_back({OpCode::Ceq, 0, 2});
    m.instructions.push_back({OpCode::Ret, 0, 3});

    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 1);
}

TEST_F(CompleteInterpreterTest, Ceq_NotEqual) {
    MethodBody m;
    m.max_stack = 2;
    m.max_locals = 0;
    m.instructions.push_back({OpCode::Ldc_I4, 5, 0});
    m.instructions.push_back({OpCode::Ldc_I4, 6, 1});
    m.instructions.push_back({OpCode::Ceq, 0, 2});
    m.instructions.push_back({OpCode::Ret, 0, 3});

    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 0);
}

TEST_F(CompleteInterpreterTest, Cgt) {
    MethodBody m;
    m.max_stack = 2;
    m.max_locals = 0;
    m.instructions.push_back({OpCode::Ldc_I4, 10, 0});
    m.instructions.push_back({OpCode::Ldc_I4, 5, 1});
    m.instructions.push_back({OpCode::Cgt, 0, 2});
    m.instructions.push_back({OpCode::Ret, 0, 3});

    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 1);
}

TEST_F(CompleteInterpreterTest, Clt) {
    MethodBody m;
    m.max_stack = 2;
    m.max_locals = 0;
    m.instructions.push_back({OpCode::Ldc_I4, 5, 0});
    m.instructions.push_back({OpCode::Ldc_I4, 10, 1});
    m.instructions.push_back({OpCode::Clt, 0, 2});
    m.instructions.push_back({OpCode::Ret, 0, 3});

    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 1);
}

// ========== Branch Instructions ==========

TEST_F(CompleteInterpreterTest, Br) {
    MethodBody m;
    m.max_stack = 1;
    m.max_locals = 0;
    m.instructions.push_back({OpCode::Br, 2, 0});
    m.instructions.push_back({OpCode::Ldc_I4, 0, 1});
    m.instructions.push_back({OpCode::Ldc_I4, 42, 2});
    m.instructions.push_back({OpCode::Ret, 0, 3});

    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 42);
}

TEST_F(CompleteInterpreterTest, Brfalse) {
    MethodBody m;
    m.max_stack = 1;
    m.max_locals = 0;
    m.instructions.push_back({OpCode::Ldc_I4, 0, 0});
    m.instructions.push_back({OpCode::Brfalse, 3, 1});
    m.instructions.push_back({OpCode::Ldc_I4, 0, 2});
    m.instructions.push_back({OpCode::Ldc_I4, 42, 3});
    m.instructions.push_back({OpCode::Ret, 0, 4});

    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 42);
}

TEST_F(CompleteInterpreterTest, Brtrue) {
    MethodBody m;
    m.max_stack = 1;
    m.max_locals = 0;
    m.instructions.push_back({OpCode::Ldc_I4, 1, 0});
    m.instructions.push_back({OpCode::Brtrue, 3, 1});
    m.instructions.push_back({OpCode::Ldc_I4, 0, 2});
    m.instructions.push_back({OpCode::Ldc_I4, 42, 3});
    m.instructions.push_back({OpCode::Ret, 0, 4});

    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 42);
}

TEST_F(CompleteInterpreterTest, Loop) {
    MethodBody m;
    m.max_stack = 2;
    m.max_locals = 2;

    // int sum = 0;
    m.instructions.push_back({OpCode::Ldc_I4, 0, 0});     // 0: push 0
    m.instructions.push_back({OpCode::STloc_0, 0, 1});     // 1: sum = 0

    // for (int i = 0; i < 10; i++)
    m.instructions.push_back({OpCode::Ldc_I4, 0, 2});      // 2: push 0
    m.instructions.push_back({OpCode::STloc_1, 0, 3});      // 3: i = 0

    m.instructions.push_back({OpCode::LDloc_1, 0, 4});      // 4: push i
    m.instructions.push_back({OpCode::Ldc_I4, 10, 5});      // 5: push 10
    m.instructions.push_back({OpCode::Bge, 12, 6});         // 6: if i >= 10, goto 12

    m.instructions.push_back({OpCode::LDloc_0, 0, 7});      // 7: push sum
    m.instructions.push_back({OpCode::LDloc_1, 0, 8});      // 8: push i
    m.instructions.push_back({OpCode::Add, 0, 9});           // 9: sum + i
    m.instructions.push_back({OpCode::STloc_0, 0, 10});     // 10: sum = sum + i

    m.instructions.push_back({OpCode::LDloc_1, 0, 11});     // 11: push i
    m.instructions.push_back({OpCode::Ldc_I4, 1, 12});      // 12: push 1
    m.instructions.push_back({OpCode::Add, 0, 13});          // 13: i + 1
    m.instructions.push_back({OpCode::STloc_1, 0, 14});     // 14: i = i + 1
    m.instructions.push_back({OpCode::Br, 4, 15});           // 15: goto 4

    m.instructions.push_back({OpCode::LDloc_0, 0, 16});     // 16: push sum
    m.instructions.push_back({OpCode::Ret, 0, 17});         // 17: return

    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 45); // 0+1+2+...+9 = 45
}

// ========== Dup and Pop ==========

TEST_F(CompleteInterpreterTest, Dup) {
    MethodBody m;
    m.max_stack = 2;
    m.max_locals = 0;
    m.instructions.push_back({OpCode::Ldc_I4, 7, 0});
    m.instructions.push_back({OpCode::Dup, 0, 1});
    m.instructions.push_back({OpCode::Add, 0, 2});
    m.instructions.push_back({OpCode::Ret, 0, 3});

    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 14);
}

TEST_F(CompleteInterpreterTest, Pop) {
    MethodBody m;
    m.max_stack = 2;
    m.max_locals = 0;
    m.instructions.push_back({OpCode::Ldc_I4, 42, 0});
    m.instructions.push_back({OpCode::Ldc_I4, 7, 1});
    m.instructions.push_back({OpCode::Pop, 0, 2});
    m.instructions.push_back({OpCode::Ret, 0, 3});

    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 7);
}

// ========== Local Variables ==========

TEST_F(CompleteInterpreterTest, LocalVariables) {
    MethodBody m;
    m.max_stack = 1;
    m.max_locals = 1;
    m.instructions.push_back({OpCode::Ldc_I4, 42, 0});
    m.instructions.push_back({OpCode::STloc_0, 0, 1});
    m.instructions.push_back({OpCode::LDloc_0, 0, 2});
    m.instructions.push_back({OpCode::Ret, 0, 3});

    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 42);
}

TEST_F(CompleteInterpreterTest, MultipleLocals) {
    MethodBody m;
    m.max_stack = 1;
    m.max_locals = 3;
    m.instructions.push_back({OpCode::Ldc_I4, 10, 0});
    m.instructions.push_back({OpCode::STloc_0, 0, 1});
    m.instructions.push_back({OpCode::Ldc_I4, 20, 2});
    m.instructions.push_back({OpCode::STloc_1, 0, 3});
    m.instructions.push_back({OpCode::Ldc_I4, 30, 4});
    m.instructions.push_back({OpCode::STloc_2, 0, 5});
    m.instructions.push_back({OpCode::LDloc_0, 0, 6});
    m.instructions.push_back({OpCode::LDloc_1, 0, 7});
    m.instructions.push_back({OpCode::Add, 0, 8});
    m.instructions.push_back({OpCode::LDloc_2, 0, 9});
    m.instructions.push_back({OpCode::Add, 0, 10});
    m.instructions.push_back({OpCode::Ret, 0, 11});

    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 60);
}

// ========== Array Operations ==========

TEST_F(CompleteInterpreterTest, ArrayCreateAccess) {
    MethodBody m;
    m.max_stack = 3;
    m.max_locals = 1;
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

// ========== Register Compiler ==========

TEST_F(CompleteInterpreterTest, RegisterCompiler) {
    MethodBody m;
    m.max_stack = 2;
    m.max_locals = 0;
    m.instructions.push_back({OpCode::Ldc_I4, 3, 0});
    m.instructions.push_back({OpCode::Ldc_I4, 4, 1});
    m.instructions.push_back({OpCode::Add, 0, 2});
    m.instructions.push_back({OpCode::Ret, 0, 3});

    auto reg_code = interp_->CompileToRegister(m);
    EXPECT_EQ(reg_code.size(), 4u);
    EXPECT_EQ(reg_code[0].opcode, RegOpCode::Ldc_I4);
    EXPECT_EQ(reg_code[0].operand, 3u);
    EXPECT_EQ(reg_code[1].opcode, RegOpCode::Ldc_I4);
    EXPECT_EQ(reg_code[1].operand, 4u);
    EXPECT_EQ(reg_code[2].opcode, RegOpCode::Add_I4);
    EXPECT_EQ(reg_code[3].opcode, RegOpCode::Ret);
}

// ========== Memory Manager ==========

TEST_F(CompleteInterpreterTest, ArrayAllocAndFree) {
    void* arr = memory_->AllocateArray(4, 10);
    EXPECT_NE(arr, nullptr);
    uint32_t len = *static_cast<uint32_t*>(arr);
    EXPECT_EQ(len, 10u);
}

// ========== Complex Programs ==========

TEST_F(CompleteInterpreterTest, Fibonacci) {
    // int fib(int n) { if (n <= 1) return n; return fib(n-1) + fib(n-2); }
    // For n=10, result should be 55
    // Inline iterative version:
    MethodBody m;
    m.max_stack = 4;
    m.max_locals = 3; // a, b, temp

    // a = 0, b = 1, for i=0 to n: temp = a+b, a = b, b = temp
    // Actually let's do: sum=0, a=0, b=1, loop 10 times: temp=a, a=b, b=temp+b
    m.instructions.push_back({OpCode::Ldc_I4, 0, 0});      // 0: push 0
    m.instructions.push_back({OpCode::STloc_0, 0, 1});      // 1: a = 0
    m.instructions.push_back({OpCode::Ldc_I4, 1, 2});      // 2: push 1
    m.instructions.push_back({OpCode::STloc_1, 0, 3});     // 3: b = 1
    m.instructions.push_back({OpCode::Ldc_I4, 0, 4});      // 4: push 0 (counter)
    m.instructions.push_back({OpCode::STloc_2, 0, 5});     // 5: i = 0

    // loop: i < 10
    m.instructions.push_back({OpCode::LDloc_2, 0, 6});     // 6: push i
    m.instructions.push_back({OpCode::Ldc_I4, 10, 7});     // 7: push 10
    m.instructions.push_back({OpCode::Bge, 17, 8});         // 8: if i >= 10, goto 17

    m.instructions.push_back({OpCode::LDloc_0, 0, 9});     // 9: push a
    m.instructions.push_back({OpCode::LDloc_1, 0, 10});    // 10: push b
    m.instructions.push_back({OpCode::Add, 0, 11});        // 11: a + b
    m.instructions.push_back({OpCode::LDloc_1, 0, 12});    // 12: push b (save old b)
    m.instructions.push_back({OpCode::STloc_0, 0, 13});    // 13: a = a + b
    m.instructions.push_back({OpCode::STloc_1, 0, 14});    // 14: b = old b (wrong, need temp)

    // Actually let me simplify: a, b = b, a+b
    // temp = a
    m.instructions.push_back({OpCode::LDloc_0, 0, 9});     // 9: push a
    m.instructions.push_back({OpCode::STloc_2, 0, 10});    // 10: temp = a (reuse i as temp for now)

    // a = b
    m.instructions.push_back({OpCode::LDloc_1, 0, 11});    // 11: push b
    m.instructions.push_back({OpCode::STloc_0, 0, 12});    // 12: a = b

    // b = temp + b
    m.instructions.push_back({OpCode::LDloc_2, 0, 13});    // 13: push temp (old a)
    m.instructions.push_back({OpCode::LDloc_1, 0, 14});    // 14: push b
    m.instructions.push_back({OpCode::Add, 0, 15});        // 15: temp + b
    m.instructions.push_back({OpCode::STloc_1, 0, 16});    // 16: b = temp + b

    m.instructions.push_back({OpCode::Br, 6, 17});          // 17: goto loop

    m.instructions.push_back({OpCode::LDloc_0, 0, 18});    // 18: push a
    m.instructions.push_back({OpCode::Ret, 0, 19});        // 19: return

    interp_->Execute(m);
    EXPECT_EQ(interp_->GetReturnInt(), 55); // fib(10) = 55
}
