#pragma once

#include "platform.h"
#include <cstdint>
#include <cstring>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <array>
#include <thread>

namespace hotc {

// Arena allocator for zero-fragmentation allocation
class ArenaAllocator {
public:
    ArenaAllocator(size_t block_size = 1024 * 1024) 
        : block_size_(block_size), current_block_(nullptr), current_offset_(0) {
        AllocateNewBlock();
    }
    
    ~ArenaAllocator() {
        for (auto block : blocks_) {
            std::free(block);
        }
    }
    
    void* Allocate(size_t size, size_t alignment = 8) {
        // Align offset
        size_t aligned_offset = (current_offset_ + alignment - 1) & ~(alignment - 1);
        
        // Check if we need a new block
        if (aligned_offset + size > block_size_) {
            AllocateNewBlock();
            aligned_offset = 0;
        }
        
        void* ptr = current_block_ + aligned_offset;
        current_offset_ = aligned_offset + size;
        total_allocated_ += size;
        
        return ptr;
    }
    
    void Reset() {
        current_offset_ = 0;
        // Keep blocks for reuse
    }
    
    size_t GetTotalAllocated() const { return total_allocated_; }
    size_t GetUsedMemory() const { return current_offset_; }
    
private:
    void AllocateNewBlock() {
        current_block_ = static_cast<uint8_t*>(std::malloc(block_size_));
        blocks_.push_back(current_block_);
        current_offset_ = 0;
    }
    
    size_t block_size_;
    uint8_t* current_block_;
    size_t current_offset_;
    std::vector<uint8_t*> blocks_;
    size_t total_allocated_ = 0;
};

// Lock-free memory pool for small allocations
template<size_t BlockSize, size_t PoolSize = 1024>
class LockFreePool {
public:
    LockFreePool() {
        // Initialize free list
        for (size_t i = 0; i < PoolSize - 1; i++) {
            free_list_[i].next = &free_list_[i + 1];
        }
        free_list_[PoolSize - 1].next = nullptr;
        head_ = &free_list_[0];
    }
    
    void* Allocate() {
        FreeNode* old_head;
        do {
            old_head = head_.load(std::memory_order_acquire);
            if (!old_head) return nullptr;
        } while (!head_.compare_exchange_weak(old_head, old_head->next, 
            std::memory_order_release, std::memory_order_acquire));
        
        return old_head;
    }
    
    void Free(void* ptr) {
        if (!ptr) return;
        
        FreeNode* node = static_cast<FreeNode*>(ptr);
        FreeNode* old_head;
        do {
            old_head = head_.load(std::memory_order_acquire);
            node->next = old_head;
        } while (!head_.compare_exchange_weak(old_head, node,
            std::memory_order_release, std::memory_order_acquire));
    }
    
    bool IsFromPool(void* ptr) const {
        uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
        uintptr_t start = reinterpret_cast<uintptr_t>(&free_list_[0]);
        uintptr_t end = start + sizeof(FreeNode) * PoolSize;
        return addr >= start && addr < end;
    }
    
private:
    struct FreeNode {
        FreeNode* next;
    };
    
    std::array<FreeNode, PoolSize> free_list_;
    std::atomic<FreeNode*> head_;
};

// High-performance memory manager
class HighPerformanceMemoryManager {
public:
    HighPerformanceMemoryManager() 
        : arena_(4 * 1024 * 1024),  // 4MB arena blocks
          small_pool_(), medium_pool_() {
    }
    
    ~HighPerformanceMemoryManager() = default;
    
    void* Allocate(size_t size) {
        if (size <= 32) {
            return small_pool_.Allocate();
        } else if (size <= 256) {
            return medium_pool_.Allocate();
        } else {
            return arena_.Allocate(size);
        }
    }
    
    void* AllocateObject(size_t size) {
        // Allocate with type header
        void* ptr = Allocate(size + sizeof(uint32_t));
        if (ptr) {
            *static_cast<uint32_t*>(ptr) = 0; // Type ID placeholder
            return static_cast<uint8_t*>(ptr) + sizeof(uint32_t);
        }
        return nullptr;
    }
    
    void* AllocateArray(size_t element_size, uint32_t count) {
        // Allocate with length header
        size_t total = sizeof(uint32_t) + element_size * count;
        void* ptr = Allocate(total);
        if (ptr) {
            *static_cast<uint32_t*>(ptr) = count;
            return static_cast<uint8_t*>(ptr) + sizeof(uint32_t);
        }
        return nullptr;
    }
    
    void Free(void* ptr) {
        // Arena doesn't free individual allocations
        // Small/medium pools use lock-free free lists
        if (small_pool_.IsFromPool(ptr)) {
            small_pool_.Free(ptr);
        } else if (medium_pool_.IsFromPool(ptr)) {
            medium_pool_.Free(ptr);
        }
        // Arena blocks are freed in destructor
    }
    
    void CollectGarbage() {
        // Arena doesn't need GC - everything is freed at once
        // Could add generational GC here for large objects
    }
    
    size_t GetTotalAllocated() const { return arena_.GetTotalAllocated(); }
    size_t GetUsedMemory() const { return arena_.GetUsedMemory(); }
    
private:
    ArenaAllocator arena_;
    LockFreePool<32, 4096> small_pool_;      // 32-byte blocks
    LockFreePool<256, 1024> medium_pool_;    // 256-byte blocks
};

} // namespace hotc
