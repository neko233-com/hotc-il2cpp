#include <gtest/gtest.h>
#include "../src/core/interpreter.h"
#include "../src/core/type_system.h"
#include "../src/core/memory.h"

using namespace hotc;

class InterpreterTest : public ::testing::Test {
protected:
    void SetUp() override {
        types_ = std::make_unique<TypeSystem>();
        memory_ = std::make_unique<MemoryManager>();
        interpreter_ = std::make_unique<Interpreter>(*types_, *memory_);
    }

    std::unique_ptr<TypeSystem> types_;
    std::unique_ptr<MemoryManager> memory_;
    std::unique_ptr<Interpreter> interpreter_;
};

TEST_F(InterpreterTest, ExecuteNop) {
    MethodBody method;
    method.max_stack = 1;
    method.max_locals = 0;
    method.instructions.push_back({OpCode::Nop, 0, 0});
    method.instructions.push_back({OpCode::Ret, 0, 1});

    EXPECT_NO_THROW(interpreter_->Execute(method));
}

TEST_F(InterpreterTest, LdcI4) {
    MethodBody method;
    method.max_stack = 1;
    method.max_locals = 0;
    method.instructions.push_back({OpCode::Ldc_I4, 42, 0});
    method.instructions.push_back({OpCode::Ret, 0, 1});

    interpreter_->Execute(method);
    EXPECT_EQ(interpreter_->GetReturnInt(), 42);
}

TEST_F(InterpreterTest, Add) {
    MethodBody method;
    method.max_stack = 2;
    method.max_locals = 0;
    method.instructions.push_back({OpCode::Ldc_I4, 3, 0});
    method.instructions.push_back({OpCode::Ldc_I4, 4, 1});
    method.instructions.push_back({OpCode::Add, 0, 2});
    method.instructions.push_back({OpCode::Ret, 0, 3});

    interpreter_->Execute(method);
    EXPECT_EQ(interpreter_->GetReturnInt(), 7);
}

TEST_F(InterpreterTest, Sub) {
    MethodBody method;
    method.max_stack = 2;
    method.max_locals = 0;
    method.instructions.push_back({OpCode::Ldc_I4, 10, 0});
    method.instructions.push_back({OpCode::Ldc_I4, 3, 1});
    method.instructions.push_back({OpCode::Sub, 0, 2});
    method.instructions.push_back({OpCode::Ret, 0, 3});

    interpreter_->Execute(method);
    EXPECT_EQ(interpreter_->GetReturnInt(), 7);
}

TEST_F(InterpreterTest, Mul) {
    MethodBody method;
    method.max_stack = 2;
    method.max_locals = 0;
    method.instructions.push_back({OpCode::Ldc_I4, 5, 0});
    method.instructions.push_back({OpCode::Ldc_I4, 6, 1});
    method.instructions.push_back({OpCode::Mul, 0, 2});
    method.instructions.push_back({OpCode::Ret, 0, 3});

    interpreter_->Execute(method);
    EXPECT_EQ(interpreter_->GetReturnInt(), 30);
}

TEST_F(InterpreterTest, Branch) {
    MethodBody method;
    method.max_stack = 1;
    method.max_locals = 0;
    method.instructions.push_back({OpCode::Ldc_I4, 0, 0});
    method.instructions.push_back({OpCode::Brfalse, 3, 1});
    method.instructions.push_back({OpCode::Ldc_I4, 1, 2});
    method.instructions.push_back({OpCode::Ldc_I4, 2, 3});
    method.instructions.push_back({OpCode::Ret, 0, 4});

    interpreter_->Execute(method);
    EXPECT_EQ(interpreter_->GetReturnInt(), 2);
}

TEST_F(InterpreterTest, LocalVariables) {
    MethodBody method;
    method.max_stack = 1;
    method.max_locals = 1;
    method.instructions.push_back({OpCode::Ldc_I4, 42, 0});
    method.instructions.push_back({OpCode::Stloc_0, 0, 1});
    method.instructions.push_back({OpCode::Ldloc_0, 0, 2});
    method.instructions.push_back({OpCode::Ret, 0, 3});

    interpreter_->Execute(method);
    EXPECT_EQ(interpreter_->GetReturnInt(), 42);
}

TEST_F(InterpreterTest, Dup) {
    MethodBody method;
    method.max_stack = 2;
    method.max_locals = 0;
    method.instructions.push_back({OpCode::Ldc_I4, 7, 0});
    method.instructions.push_back({OpCode::Dup, 0, 1});
    method.instructions.push_back({OpCode::Add, 0, 2});
    method.instructions.push_back({OpCode::Ret, 0, 3});

    interpreter_->Execute(method);
    EXPECT_EQ(interpreter_->GetReturnInt(), 14);
}
