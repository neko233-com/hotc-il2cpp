#include <gtest/gtest.h>
#include "../src/core/memory.h"

using namespace hotc;

class MemoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        memory_ = std::make_unique<MemoryManager>();
    }

    std::unique_ptr<MemoryManager> memory_;
};

TEST_F(MemoryTest, Allocate) {
    void* ptr = memory_->Allocate(64);
    EXPECT_NE(ptr, nullptr);
    EXPECT_GT(memory_->GetUsedMemory(), 0);
}

TEST_F(MemoryTest, AllocateAndFree) {
    void* ptr = memory_->Allocate(64);
    EXPECT_NE(ptr, nullptr);

    size_t used_before = memory_->GetUsedMemory();
    memory_->Free(ptr);
    EXPECT_LT(memory_->GetUsedMemory(), used_before);
}

TEST_F(MemoryTest, MultipleAllocations) {
    void* p1 = memory_->Allocate(64);
    void* p2 = memory_->Allocate(128);
    void* p3 = memory_->Allocate(256);

    EXPECT_NE(p1, nullptr);
    EXPECT_NE(p2, nullptr);
    EXPECT_NE(p3, nullptr);
    EXPECT_NE(p1, p2);
    EXPECT_NE(p2, p3);
}

TEST_F(MemoryTest, AllocateObject) {
    void* obj = memory_->AllocateObject(32);
    EXPECT_NE(obj, nullptr);
}

TEST_F(MemoryTest, AllocateArray) {
    void* arr = memory_->AllocateArray(4, 10);
    EXPECT_NE(arr, nullptr);
}

TEST_F(MemoryTest, GCThreshold) {
    memory_->SetGCThreshold(1024);

    for (int i = 0; i < 100; i++) {
        void* ptr = memory_->Allocate(64);
        memory_->Free(ptr);
    }

    EXPECT_LT(memory_->GetUsedMemory(), 1024);
}
