#pragma once

#include <chrono>
#include <functional>
#include <string>
#include <vector>
#include <cstdio>
#include <algorithm>
#include <numeric>

namespace hotc {

struct BenchmarkResult {
    std::string name;
    double elapsed_ms;
    size_t iterations;
    double ops_per_sec;
    double avg_ns_per_op;
    double min_ns;
    double max_ns;
    double p50_ns;
    double p95_ns;
    double p99_ns;
};

class Benchmark {
public:
    static Benchmark& Instance();
    
    void Run(const std::string& name, std::function<void()> func, size_t iterations = 1000000);
    void Run(const std::string& name, std::function<void(int)> func, size_t iterations = 1000000);
    void Run(const std::string& name, std::function<void(void*)> func, void* context, size_t iterations = 1000000);
    
    void PrintResults();
    void ExportJSON(const std::string& path);
    void ExportHTML(const std::string& path);
    void ExportMarkdown(const std::string& path);
    
    const std::vector<BenchmarkResult>& GetResults() const { return results_; }
    void Clear() { results_.clear(); }

private:
    Benchmark() = default;
    
    void AddResult(const std::string& name, std::vector<double>& timings, size_t iterations);
    
    std::vector<BenchmarkResult> results_;
};

// Benchmark suite for IL2CPP interpreter
class IL2CPPBenchmarkSuite {
public:
    IL2CPPBenchmarkSuite() = default;
    ~IL2CPPBenchmarkSuite() = default;
    
    void RunAll();
    
    // Arithmetic benchmarks
    void BenchmarkInt32Add();
    void BenchmarkInt32Sub();
    void BenchmarkInt32Mul();
    void BenchmarkInt32Div();
    void BenchmarkInt32Rem();
    void BenchmarkInt32And();
    void BenchmarkInt32Or();
    void BenchmarkInt32Xor();
    void BenchmarkInt32Shl();
    void BenchmarkInt32Shr();
    void BenchmarkInt32Neg();
    void BenchmarkInt32Not();
    
    // Comparison benchmarks
    void BenchmarkInt32Ceq();
    void BenchmarkInt32Cgt();
    void BenchmarkInt32Clt();
    
    // Branch benchmarks
    void BenchmarkBranchUnconditional();
    void BenchmarkBranchConditional();
    void BenchmarkBranchCompare();
    void BenchmarkBranchLoop();
    void BenchmarkBranchNested();
    
    // Stack operation benchmarks
    void BenchmarkPushPop();
    void BenchmarkDup();
    void BenchmarkSwap();
    
    // Memory benchmarks
    void BenchmarkAllocSmall();
    void BenchmarkAllocMedium();
    void BenchmarkAllocLarge();
    void BenchmarkFree();
    
    // Type system benchmarks
    void BenchmarkTypeLookup();
    void BenchmarkTypeCheck();
    void BenchmarkVirtualDispatch();
    
    // Method call benchmarks
    void BenchmarkDirectCall();
    void BenchmarkVirtualCall();
    void BenchmarkInterfaceCall();
    void BenchmarkDelegateCall();
    
    // Object benchmarks
    void BenchmarkObjectCreate();
    void BenchmarkObjectAccess();
    void BenchmarkArrayAccess();
    void BenchmarkStringOps();
    
    // Exception handling benchmarks
    void BenchmarkTryCatch();
    void BenchmarkFinally();
    void BenchmarkExceptionThrow();
    
    // Real-world scenario benchmarks
    void BenchmarkUnityGameObjectCreation();
    void BenchmarkUnityTransformAccess();
    void BenchmarkUnityComponentLookup();
    void BenchmarkUnityPhysicsRaycast();
    void BenchmarkUnityUICallback();
    
    // Stress tests
    void BenchmarkConcurrentAllocation();
    void BenchmarkLargeMethodExecution();
    void BenchmarkDeepCallStack();
    void BenchmarkHighBranchRate();
    
    void PrintResults();
    void ExportJSON(const std::string& path);
    void ExportHTML(const std::string& path);
    
private:
    Benchmark suite_;
};

} // namespace hotc
