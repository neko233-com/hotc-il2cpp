#pragma once

#include <cstdint>
#include <vector>
#include <memory>

namespace hotc {

class MemoryManager {
public:
    MemoryManager();
    ~MemoryManager() = default;

    void* Allocate(size_t size);
    void* AllocateObject(size_t size);
    void* AllocateArray(size_t element_size, uint32_t count);
    void Free(void* ptr);

    void CollectGarbage();
    void SetGCThreshold(size_t threshold);

    size_t GetTotalAllocated() const { return total_allocated_; }
    size_t GetUsedMemory() const { return used_memory_; }

private:
    struct Block {
        uint8_t* data;
        size_t size;
        bool in_use;
        size_t alloc_size = 0;
    };

    std::vector<Block> blocks_;
    size_t total_allocated_ = 0;
    size_t used_memory_ = 0;
    size_t gc_threshold_ = 1024 * 1024; // 1MB default

    void ExpandHeap(size_t size);
};

} // namespace hotc
