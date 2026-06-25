#pragma once

#include <chrono>
#include <functional>
#include <string>
#include <vector>
#include <cstdio>

namespace hotc {

struct BenchmarkResult {
    std::string name;
    double elapsed_ms;
    size_t iterations;
    double ops_per_sec;
};

class Benchmark {
public:
    static Benchmark& Instance();
    
    void Run(const std::string& name, std::function<void()> func, size_t iterations = 1000000);
    void Run(const std::string& name, std::function<void(int)> func, size_t iterations = 1000000);
    
    void PrintResults();
    void ExportJSON(const std::string& path);
    void ExportHTML(const std::string& path);
    
    const std::vector<BenchmarkResult>& GetResults() const { return results_; }

private:
    Benchmark() = default;
    std::vector<BenchmarkResult> results_;
};

} // namespace hotc
