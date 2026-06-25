#include "memory.h"
#include <cstdlib>
#include <cstring>
#include <algorithm>

namespace hotc {

MemoryManager::MemoryManager() {
    ExpandHeap(1024 * 1024); // Initial 1MB
}

void* MemoryManager::Allocate(size_t size) {
    for (auto& block : blocks_) {
        if (!block.in_use && block.size >= size) {
            block.in_use = true;
            block.alloc_size = size;
            used_memory_ += size;
            return block.data;
        }
    }

    ExpandHeap(size);
    Block& new_block = blocks_.back();
    new_block.in_use = true;
    new_block.alloc_size = size;
    used_memory_ += size;
    return new_block.data;
}

void* MemoryManager::AllocateObject(size_t size) {
    return Allocate(size + sizeof(uint32_t)); // Extra space for type header
}

void* MemoryManager::AllocateArray(size_t element_size, uint32_t count) {
    size_t total = sizeof(uint32_t) + element_size * count; // length + elements
    return Allocate(total);
}

void MemoryManager::Free(void* ptr) {
    if (!ptr) return;

    for (auto& block : blocks_) {
        if (block.data == ptr) {
            block.in_use = false;
            used_memory_ -= block.alloc_size;
            block.alloc_size = 0;
            break;
        }
    }

    if (used_memory_ < gc_threshold_ / 2) {
        CollectGarbage();
    }
}

void MemoryManager::CollectGarbage() {
    std::vector<Block> merged;
    Block current = blocks_[0];

    for (size_t i = 1; i < blocks_.size(); i++) {
        if (!blocks_[i].in_use && !current.in_use &&
            current.data + current.size == blocks_[i].data) {
            current.size += blocks_[i].size;
        } else {
            merged.push_back(current);
            current = blocks_[i];
        }
    }
    merged.push_back(current);

    blocks_ = std::move(merged);
}

void MemoryManager::SetGCThreshold(size_t threshold) {
    gc_threshold_ = threshold;
}

void MemoryManager::ExpandHeap(size_t size) {
    size_t alloc_size = std::max(size, static_cast<size_t>(1024 * 1024));
    uint8_t* data = static_cast<uint8_t*>(std::malloc(alloc_size));

    Block block;
    block.data = data;
    block.size = alloc_size;
    block.in_use = false;

    blocks_.push_back(block);
    total_allocated_ += alloc_size;
}

} // namespace hotc
