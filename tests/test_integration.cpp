#pragma once

#include <gtest/gtest.h>
#include "../src/core/hp_interpreter.h"
#include "../src/core/hp_memory.h"
#include "../src/core/type_system.h"
#include "../src/core/memory.h"
#include "../src/core/il_parser.h"
#include "../src/core/il2cpp_metadata.h"
#include "../src/core/exception_handling.h"
#include "../src/core/jit_compiler.h"
#include "../src/bridge/unity_bridge.h"
#include <memory>

namespace hotc {

class HPInterpreterTest : public ::testing::Test {
protected:
    void SetUp() override {
        types_ = std::make_unique<TypeSystem>();
        memory_ = std::make_unique<MemoryManager>();
        hp_memory_ = std::make_unique<HighPerformanceMemoryManager>();
        metadata_ = std::make_unique<IL2CPPMetadataLoader>();
        interp_ = std::make_unique<HighPerformanceInterpreter>(*types_, *memory_);
    }
    
    std::unique_ptr<TypeSystem> types_;
    std::unique_ptr<MemoryManager> memory_;
    std::unique_ptr<HighPerformanceMemoryManager> hp_memory_;
    std::unique_ptr<IL2CPPMetadataLoader> metadata_;
    std::unique_ptr<HighPerformanceInterpreter> interp_;
};

class HPMemoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        mem_ = std::make_unique<HighPerformanceMemoryManager>();
    }
    
    std::unique_ptr<HighPerformanceMemoryManager> mem_;
};

class ExceptionHandlingTest : public ::testing::Test {
protected:
    void SetUp() override {
        handler_ = std::make_unique<ExceptionHandler>();
    }
    
    std::unique_ptr<ExceptionHandler> handler_;
};

class JITCompilerTest : public ::testing::Test {
protected:
    void SetUp() override {
        compiler_ = std::make_unique<JITCompiler>();
    }
    
    std::unique_ptr<JITCompiler> compiler_;
};

class UnityBridgeTest : public ::testing::Test {
protected:
    void SetUp() override {
        types_ = std::make_unique<TypeSystem>();
        memory_ = std::make_unique<MemoryManager>();
        metadata_ = std::make_unique<IL2CPPMetadataLoader>();
        interp_ = std::make_unique<HighPerformanceInterpreter>(*types_, *memory_);
        bridge_ = std::make_unique<UnityBridge>(*interp_, *metadata_);
    }
    
    std::unique_ptr<TypeSystem> types_;
    std::unique_ptr<MemoryManager> memory_;
    std::unique_ptr<IL2CPPMetadataLoader> metadata_;
    std::unique_ptr<HighPerformanceInterpreter> interp_;
    std::unique_ptr<UnityBridge> bridge_;
};

// HP Interpreter Tests

TEST_F(HPInterpreterTest, ExecuteNop) {
    MethodBody method;
    method.max_stack = 1;
    method.max_locals = 0;
    method.instructions.push_back({OpCode::Nop, 0, 0});
    method.instructions.push_back({OpCode::Ret, 0, 1});
    
    EXPECT_NO_THROW(interp_->Execute(method));
}

TEST_F(HPInterpreterTest, LdcI4) {
    MethodBody method;
    method.max_stack = 1;
    method.max_locals = 0;
    method.instructions.push_back({OpCode::Ldc_I4, 42, 0});
    method.instructions.push_back({OpCode::Ret, 0, 1});
    
    interp_->Execute(method);
    EXPECT_EQ(interp_->GetReturnInt(), 42);
}

TEST_F(HPInterpreterTest, Add) {
    MethodBody method;
    method.max_stack = 2;
    method.max_locals = 0;
    method.instructions.push_back({OpCode::Ldc_I4, 3, 0});
    method.instructions.push_back({OpCode::Ldc_I4, 4, 1});
    method.instructions.push_back({OpCode::Add, 0, 2});
    method.instructions.push_back({OpCode::Ret, 0, 3});
    
    interp_->Execute(method);
    EXPECT_EQ(interp_->GetReturnInt(), 7);
}

TEST_F(HPInterpreterTest, Sub) {
    MethodBody method;
    method.max_stack = 2;
    method.max_locals = 0;
    method.instructions.push_back({OpCode::Ldc_I4, 10, 0});
    method.instructions.push_back({OpCode::Ldc_I4, 3, 1});
    method.instructions.push_back({OpCode::Sub, 0, 2});
    method.instructions.push_back({OpCode::Ret, 0, 3});
    
    interp_->Execute(method);
    EXPECT_EQ(interp_->GetReturnInt(), 7);
}

TEST_F(HPInterpreterTest, Mul) {
    MethodBody method;
    method.max_stack = 2;
    method.max_locals = 0;
    method.instructions.push_back({OpCode::Ldc_I4, 5, 0});
    method.instructions.push_back({OpCode::Ldc_I4, 6, 1});
    method.instructions.push_back({OpCode::Mul, 0, 2});
    method.instructions.push_back({OpCode::Ret, 0, 3});
    
    interp_->Execute(method);
    EXPECT_EQ(interp_->GetReturnInt(), 30);
}

TEST_F(HPInterpreterTest, Dup) {
    MethodBody method;
    method.max_stack = 2;
    method.max_locals = 0;
    method.instructions.push_back({OpCode::Ldc_I4, 7, 0});
    method.instructions.push_back({OpCode::Dup, 0, 1});
    method.instructions.push_back({OpCode::Add, 0, 2});
    method.instructions.push_back({OpCode::Ret, 0, 3});
    
    interp_->Execute(method);
    EXPECT_EQ(interp_->GetReturnInt(), 14);
}

// HP Memory Tests

TEST_F(HPMemoryTest, AllocateSmall) {
    void* ptr = mem_->Allocate(32);
    EXPECT_NE(ptr, nullptr);
}

TEST_F(HPMemoryTest, AllocateMedium) {
    void* ptr = mem_->Allocate(256);
    EXPECT_NE(ptr, nullptr);
}

TEST_F(HPMemoryTest, AllocateLarge) {
    void* ptr = mem_->Allocate(4096);
    EXPECT_NE(ptr, nullptr);
}

TEST_F(HPMemoryTest, AllocateObject) {
    void* ptr = mem_->AllocateObject(32);
    EXPECT_NE(ptr, nullptr);
}

TEST_F(HPMemoryTest, AllocateArray) {
    void* ptr = mem_->AllocateArray(4, 10);
    EXPECT_NE(ptr, nullptr);
}

TEST_F(HPMemoryTest, MultipleAllocations) {
    void* p1 = mem_->Allocate(32);
    void* p2 = mem_->Allocate(32);
    void* p3 = mem_->Allocate(32);
    
    EXPECT_NE(p1, nullptr);
    EXPECT_NE(p2, nullptr);
    EXPECT_NE(p3, nullptr);
    EXPECT_NE(p1, p2);
    EXPECT_NE(p2, p3);
}

// Exception Handling Tests

TEST_F(ExceptionHandlingTest, CreateNullReference) {
    auto* exc = ExceptionHandler::CreateNullReference();
    EXPECT_NE(exc, nullptr);
    EXPECT_STREQ(exc->class_name, "System.NullReferenceException");
    delete exc;
}

TEST_F(ExceptionHandlingTest, CreateInvalidCast) {
    auto* exc = ExceptionHandler::CreateInvalidCast();
    EXPECT_NE(exc, nullptr);
    EXPECT_STREQ(exc->class_name, "System.InvalidCastException");
    delete exc;
}

TEST_F(ExceptionHandlingTest, CreateOverflow) {
    auto* exc = ExceptionHandler::CreateOverflow();
    EXPECT_NE(exc, nullptr);
    EXPECT_STREQ(exc->class_name, "System.OverflowException");
    delete exc;
}

TEST_F(ExceptionHandlingTest, CreateDivideByZero) {
    auto* exc = ExceptionHandler::CreateDivideByZero();
    EXPECT_NE(exc, nullptr);
    EXPECT_STREQ(exc->class_name, "System.DivideByZeroException");
    delete exc;
}

TEST_F(ExceptionHandlingTest, PushPopFrame) {
    handler_->PushFrame(0, 10, 10, 20, 0, ExceptionType::None, 0);
    EXPECT_TRUE(handler_->HasException() == false);
    handler_->PopFrame();
}

TEST_F(ExceptionHandlingTest, FindHandler) {
    handler_->PushFrame(0, 10, 10, 20, 0, ExceptionType::NullReference, 0);
    auto* frame = handler_->FindHandler(5, ExceptionType::NullReference, 0);
    EXPECT_NE(frame, nullptr);
    handler_->PopFrame();
}

// JIT Compiler Tests

TEST_F(JITCompilerTest, ShouldNotCompileCold) {
    EXPECT_FALSE(compiler_->ShouldCompile(0, 100));
}

TEST_F(JITCompilerTest, ShouldCompileHot) {
    EXPECT_TRUE(compiler_->ShouldCompile(0, 100000));
}

// Unity Bridge Tests

TEST_F(UnityBridgeTest, CreateGameObject) {
    uint32_t id = bridge_->CreateGameObject("TestObject");
    EXPECT_GT(id, 0u);
    EXPECT_NE(bridge_->GetGameObject(id), nullptr);
}

TEST_F(UnityBridgeTest, DestroyGameObject) {
    uint32_t id = bridge_->CreateGameObject("ToDelete");
    bridge_->DestroyGameObject(id);
    EXPECT_EQ(bridge_->GetGameObject(id), nullptr);
}

TEST_F(UnityBridgeTest, SetPosition) {
    uint32_t id = bridge_->CreateGameObject("PositionTest");
    bridge_->SetPosition(id, 1.0f, 2.0f, 3.0f);
    
    float x, y, z;
    bridge_->GetPosition(id, x, y, z);
    EXPECT_FLOAT_EQ(x, 1.0f);
    EXPECT_FLOAT_EQ(y, 2.0f);
    EXPECT_FLOAT_EQ(z, 3.0f);
}

TEST_F(UnityBridgeTest, SetRotation) {
    uint32_t id = bridge_->CreateGameObject("RotationTest");
    bridge_->SetRotation(id, 0.0f, 0.0f, 0.0f, 1.0f);
    
    float x, y, z, w;
    bridge_->GetRotation(id, x, y, z, w);
    EXPECT_FLOAT_EQ(x, 0.0f);
    EXPECT_FLOAT_EQ(y, 0.0f);
    EXPECT_FLOAT_EQ(z, 0.0f);
    EXPECT_FLOAT_EQ(w, 1.0f);
}

TEST_F(UnityBridgeTest, SetScale) {
    uint32_t id = bridge_->CreateGameObject("ScaleTest");
    bridge_->SetScale(id, 2.0f, 2.0f, 2.0f);
    
    float x, y, z;
    bridge_->GetScale(id, x, y, z);
    EXPECT_FLOAT_EQ(x, 2.0f);
    EXPECT_FLOAT_EQ(y, 2.0f);
    EXPECT_FLOAT_EQ(z, 2.0f);
}

TEST_F(UnityBridgeTest, AddComponent) {
    uint32_t go_id = bridge_->CreateGameObject("WithComponent");
    uint32_t comp_id = bridge_->AddComponent(go_id, 1);
    EXPECT_GT(comp_id, 0u);
}

TEST_F(UnityBridgeTest, ObjectCount) {
    EXPECT_EQ(bridge_->GetObjectCount(), 0u);
    uint32_t id = bridge_->CreateGameObject("CountTest");
    EXPECT_GT(bridge_->GetObjectCount(), 0u);
}

} // namespace hotc
