#pragma once

#include "platform.h"
#include <cstdint>
#include <cstring>
#include <atomic>
#include <array>

namespace hotc {

// Cache line size for padding
constexpr size_t CACHE_LINE_SIZE = 64;

// Inline caching for type checks
struct InlineCache {
    static constexpr int MAX_CACHE_ENTRIES = 4;
    
    struct Entry {
        uint32_t type_id;
        uint32_t version;
    };
    
    std::array<Entry, MAX_CACHE_ENTRIES> entries;
    uint32_t hit_count;
    uint32_t miss_count;
    
    void Reset() {
        entries = {};
        hit_count = 0;
        miss_count = 0;
    }
    
    bool Check(uint32_t type_id) const {
        for (const auto& e : entries) {
            if (e.type_id == type_id) return true;
        }
        return false;
    }
    
    void RecordHit(uint32_t type_id) {
        hit_count++;
        for (auto& e : entries) {
            if (e.type_id == type_id) return;
            if (e.type_id == 0) {
                e.type_id = type_id;
                return;
            }
        }
    }
    
    void RecordMiss() { miss_count++; }
};

// Lock-free work-stealing deque for parallel execution
template<typename T, size_t Capacity = 1024>
class WorkStealingDeque {
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be power of 2");
    
    alignas(CACHE_LINE_SIZE) std::atomic<size_t> top_;
    alignas(CACHE_LINE_SIZE) std::atomic<size_t> bottom_;
    alignas(CACHE_LINE_SIZE) std::array<T, Capacity> buffer_;
    
public:
    WorkStealingDeque() : top_(0), bottom_(0) {}
    
    bool Push(T item) {
        size_t b = bottom_.load(std::memory_order_relaxed);
        size_t t = top_.load(std::memory_order_acquire);
        if (b - t >= Capacity) return false;
        buffer_[b & (Capacity - 1)] = item;
        bottom_.store(b + 1, std::memory_order_release);
        return true;
    }
    
    bool Pop(T& item) {
        size_t b = bottom_.load(std::memory_order_relaxed) - 1;
        bottom_.store(b, std::memory_order_relaxed);
        std::atomic_thread_fence(std::memory_order_seq_cst);
        size_t t = top_.load(std::memory_order_relaxed);
        if (t <= b) {
            item = buffer_[b & (Capacity - 1)];
            if (t == b) {
                if (!top_.compare_exchange_strong(t, t + 1, std::memory_order_release)) {
                    bottom_.store(t + 1, std::memory_order_relaxed);
                    return false;
                }
            }
            return true;
        }
        bottom_.store(t, std::memory_order_relaxed);
        return false;
    }
    
    bool Steal(T& item) {
        size_t t = top_.load(std::memory_order_acquire);
        std::atomic_thread_fence(std::memory_order_seq_cst);
        size_t b = bottom_.load(std::memory_order_acquire);
        if (t < b) {
            item = buffer_[t & (Capacity - 1)];
            if (top_.compare_exchange_strong(t, t + 1, std::memory_order_release)) {
                return true;
            }
        }
        return false;
    }
};

// SIMD-optimized value operations
#if defined(HOTC_HAS_SSE2)
#include <emmintrin.h>
#endif
#if defined(HOTC_HAS_NEON)
#include <arm_neon.h>
#endif

struct alignas(16) SIMDValue {
    union {
        int32_t i32[4];
        float f32[4];
        uint8_t bytes[16];
    };
    
    static SIMDValue Zero() { return {}; }
    
    static SIMDValue FromInt32(int32_t v) {
        SIMDValue val;
        val.i32[0] = v;
        val.i32[1] = 0;
        val.i32[2] = 0;
        val.i32[3] = 0;
        return val;
    }
    
    static SIMDValue FromFloat(float v) {
        SIMDValue val;
        val.f32[0] = v;
        val.f32[1] = 0;
        val.f32[2] = 0;
        val.f32[3] = 0;
        return val;
    }
};

// Thread-local execution context for zero-overhead context switching
struct alignas(CACHE_LINE_SIZE) ThreadContext {
    // Instruction pointer
    uint32_t pc;
    
    // Value stacks (separate for cache efficiency)
    alignas(CACHE_LINE_SIZE) int32_t* int_stack;
    uint32_t int_stack_top;
    uint32_t int_stack_capacity;
    
    alignas(CACHE_LINE_SIZE) int64_t* long_stack;
    uint32_t long_stack_top;
    
    alignas(CACHE_LINE_SIZE) float* float_stack;
    uint32_t float_stack_top;
    
    alignas(CACHE_LINE_SIZE) double* double_stack;
    uint32_t double_stack_top;
    
    alignas(CACHE_LINE_SIZE) void** object_stack;
    uint32_t object_stack_top;
    uint32_t object_stack_capacity;
    
    // Local variables
    alignas(CACHE_LINE_SIZE) void** locals;
    uint32_t num_locals;
    
    // Execution state
    bool running;
    bool has_exception;
    void* exception_obj;
    
    // Performance counters
    uint64_t instructions_executed;
    uint64_t branch_mispredicts;
    uint64_t cache_hits;
    uint64_t cache_misses;
    
    ThreadContext() {
        std::memset(this, 0, sizeof(*this));
        int_stack_capacity = 1024;
        int_stack = new int32_t[int_stack_capacity];
        object_stack_capacity = 256;
        object_stack = new void*[object_stack_capacity];
        num_locals = 256;
        locals = new void*[num_locals];
        float_stack = new float[256];
        double_stack = new double[256];
        long_stack = new int64_t[256];
    }
    
    ~ThreadContext() {
        delete[] int_stack;
        delete[] object_stack;
        delete[] locals;
        delete[] float_stack;
        delete[] double_stack;
        delete[] long_stack;
    }
    
    // Inline stack operations for maximum performance
    void PushInt32(int32_t v) {
        int_stack[int_stack_top++] = v;
    }
    
    int32_t PopInt32() {
        return int_stack[--int_stack_top];
    }
    
    void PushObject(void* v) {
        object_stack[object_stack_top++] = v;
    }
    
    void* PopObject() {
        return object_stack[--object_stack_top];
    }
    
    void PushFloat(float v) {
        float_stack[float_stack_top++] = v;
    }
    
    float PopFloat() {
        return float_stack[--float_stack_top];
    }
    
    void PushDouble(double v) {
        double_stack[double_stack_top++] = v;
    }
    
    double PopDouble() {
        return double_stack[--double_stack_top];
    }
    
    void PushInt64(int64_t v) {
        long_stack[long_stack_top++] = v;
    }
    
    int64_t PopInt64() {
        return long_stack[--long_stack_top];
    }
};

} // namespace hotc
