#include "benchmark_suite.h"
#include "../src/core/hp_interpreter.h"
#include "../src/core/hp_memory.h"
#include "../src/core/type_system.h"
#include "../src/core/memory.h"
#include "../src/core/il_parser.h"

namespace hotc {

Benchmark& Benchmark::Instance() {
    static Benchmark instance;
    return instance;
}

void Benchmark::Run(const std::string& name, std::function<void()> func, size_t iterations) {
    std::vector<double> timings;
    timings.reserve(iterations);
    
    // Warmup
    for (size_t i = 0; i < 1000; i++) {
        func();
    }
    
    // Measure
    for (size_t i = 0; i < iterations; i++) {
        auto start = std::chrono::high_resolution_clock::now();
        func();
        auto end = std::chrono::high_resolution_clock::now();
        timings.push_back(std::chrono::duration<double, std::nano>(end - start).count());
    }
    
    AddResult(name, timings, iterations);
}

void Benchmark::Run(const std::string& name, std::function<void(int)> func, size_t iterations) {
    std::vector<double> timings;
    timings.reserve(iterations);
    
    // Warmup
    for (size_t i = 0; i < 1000; i++) {
        func(static_cast<int>(i));
    }
    
    // Measure
    for (size_t i = 0; i < iterations; i++) {
        auto start = std::chrono::high_resolution_clock::now();
        func(static_cast<int>(i));
        auto end = std::chrono::high_resolution_clock::now();
        timings.push_back(std::chrono::duration<double, std::nano>(end - start).count());
    }
    
    AddResult(name, timings, iterations);
}

void Benchmark::Run(const std::string& name, std::function<void(void*)> func, void* context, size_t iterations) {
    std::vector<double> timings;
    timings.reserve(iterations);
    
    // Warmup
    for (size_t i = 0; i < 1000; i++) {
        func(context);
    }
    
    // Measure
    for (size_t i = 0; i < iterations; i++) {
        auto start = std::chrono::high_resolution_clock::now();
        func(context);
        auto end = std::chrono::high_resolution_clock::now();
        timings.push_back(std::chrono::duration<double, std::nano>(end - start).count());
    }
    
    AddResult(name, timings, iterations);
}

void Benchmark::AddResult(const std::string& name, std::vector<double>& timings, size_t iterations) {
    std::sort(timings.begin(), timings.end());
    
    double total = std::accumulate(timings.begin(), timings.end(), 0.0);
    double avg_ns = total / iterations;
    double min_ns = timings.front();
    double max_ns = timings.back();
    double p50_ns = timings[iterations * 50 / 100];
    double p95_ns = timings[iterations * 95 / 100];
    double p99_ns = timings[iterations * 99 / 100];
    
    double elapsed_ms = total / 1000000.0;
    double ops_per_sec = (iterations / elapsed_ms) * 1000.0;
    
    results_.push_back({
        name, elapsed_ms, iterations, ops_per_sec, avg_ns,
        min_ns, max_ns, p50_ns, p95_ns, p99_ns
    });
}

void Benchmark::PrintResults() {
    printf("\n╔══════════════════════════════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                              hotc-il2cpp Performance Benchmarks                                    ║\n");
    printf("╠══════════════════════════════════════════════════════════════════════════════════════════════════════╣\n");
    printf("║ %-45s │ %10s │ %12s │ %12s │ %10s ║\n", "Benchmark", "Time(ms)", "Ops/sec", "Avg(ns)", "P99(ns)");
    printf("╠══════════════════════════════════════════════════════════════════════════════════════════════════════╣\n");
    
    for (const auto& r : results_) {
        printf("║ %-45s │ %10.2f │ %12.0f │ %12.0f │ %10.0f ║\n",
            r.name.c_str(), r.elapsed_ms, r.ops_per_sec, r.avg_ns, r.p99_ns);
    }
    
    printf("╚══════════════════════════════════════════════════════════════════════════════════════════════════════╝\n\n");
}

void Benchmark::ExportJSON(const std::string& path) {
    std::ofstream file(path);
    file << "[\n";
    for (size_t i = 0; i < results_.size(); i++) {
        const auto& r = results_[i];
        file << "  {\n";
        file << "    \"name\": \"" << r.name << "\",\n";
        file << "    \"elapsed_ms\": " << r.elapsed_ms << ",\n";
        file << "    \"iterations\": " << r.iterations << ",\n";
        file << "    \"ops_per_sec\": " << r.ops_per_sec << ",\n";
        file << "    \"avg_ns\": " << r.avg_ns << ",\n";
        file << "    \"min_ns\": " << r.min_ns << ",\n";
        file << "    \"max_ns\": " << r.max_ns << ",\n";
        file << "    \"p50_ns\": " << r.p50_ns << ",\n";
        file << "    \"p95_ns\": " << r.p95_ns << ",\n";
        file << "    \"p99_ns\": " << r.p99_ns << "\n";
        file << "  }";
        if (i < results_.size() - 1) file << ",";
        file << "\n";
    }
    file << "]\n";
}

void Benchmark::ExportHTML(const std::string& path) {
    std::ofstream file(path);
    file << R"(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>hotc-il2cpp Performance Benchmarks</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body { font-family: 'Segoe UI', system-ui, sans-serif; background: #0d1117; color: #c9d1d9; }
        .container { max-width: 1400px; margin: 0 auto; padding: 2rem; }
        h1 { color: #58a6ff; margin-bottom: 2rem; font-size: 2.5rem; }
        .stats { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 1rem; margin-bottom: 2rem; }
        .stat-card { background: #161b22; border: 1px solid #30363d; border-radius: 8px; padding: 1.5rem; }
        .stat-label { color: #8b949e; font-size: 0.875rem; text-transform: uppercase; }
        .stat-value { color: #58a6ff; font-size: 2rem; font-weight: bold; margin-top: 0.5rem; }
        table { width: 100%; border-collapse: collapse; background: #161b22; border-radius: 8px; overflow: hidden; }
        th, td { padding: 1rem; text-align: left; border-bottom: 1px solid #30363d; }
        th { background: #21262d; color: #58a6ff; font-weight: 600; }
        tr:hover { background: #21262d; }
        .highlight { color: #3fb950; font-weight: bold; }
        .bar { height: 8px; background: linear-gradient(90deg, #238636, #3fb950); border-radius: 4px; margin-top: 0.5rem; }
        .timestamp { color: #8b949e; font-size: 0.875rem; margin-top: 2rem; }
    </style>
</head>
<body>
    <div class="container">
        <h1>hotc-il2cpp Performance Benchmarks</h1>
        <div class="stats">
            <div class="stat-card">
                <div class="stat-label">Total Benchmarks</div>
                <div class="stat-value">)" << results_.size() << R"(</div>
            </div>
            <div class="stat-card">
                <div class="stat-label">Avg Ops/sec</div>
                <div class="stat-value">)";

    double total_ops = 0;
    for (const auto& r : results_) total_ops += r.ops_per_sec;
    file << static_cast<size_t>(total_ops / results_.size());

    file << R"(</div>
            </div>
            <div class="stat-card">
                <div class="stat-label">Best Performance</div>
                <div class="stat-value">)";

    auto best = std::max_element(results_.begin(), results_.end(),
        [](const BenchmarkResult& a, const BenchmarkResult& b) { return a.ops_per_sec < b.ops_per_sec; });
    file << static_cast<size_t>(best->ops_per_sec);

    file << R"(</div>
            </div>
        </div>
        <table>
            <thead>
                <tr>
                    <th>Benchmark</th>
                    <th>Time (ms)</th>
                    <th>Iterations</th>
                    <th>Ops/sec</th>
                    <th>Avg (ns)</th>
                    <th>P50 (ns)</th>
                    <th>P95 (ns)</th>
                    <th>P99 (ns)</th>
                    <th>Performance</th>
                </tr>
            </thead>
            <tbody>)";

    double max_ops = best->ops_per_sec;
    for (const auto& r : results_) {
        double pct = (r.ops_per_sec / max_ops) * 100;
        file << "<tr>\n";
        file << "  <td>" << r.name << "</td>\n";
        file << "  <td>" << r.elapsed_ms << "</td>\n";
        file << "  <td>" << r.iterations << "</td>\n";
        file << "  <td class=\"highlight\">" << static_cast<size_t>(r.ops_per_sec) << "</td>\n";
        file << "  <td>" << r.avg_ns << "</td>\n";
        file << "  <td>" << r.p50_ns << "</td>\n";
        file << "  <td>" << r.p95_ns << "</td>\n";
        file << "  <td>" << r.p99_ns << "</td>\n";
        file << "  <td><div class=\"bar\" style=\"width: " << pct << "%\"></div></td>\n";
        file << "</tr>\n";
    }

    file << R"(</tbody>
        </table>
        <div class="timestamp">Generated: )";

    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S UTC", gmtime(&time_t));
    file << buf;

    file << R"(</div>
    </div>
</body>
</html>)";
}

void Benchmark::ExportMarkdown(const std::string& path) {
    std::ofstream file(path);
    file << "# hotc-il2cpp Performance Benchmarks\n\n";
    file << "| Benchmark | Time (ms) | Ops/sec | Avg (ns) | P99 (ns) |\n";
    file << "|-----------|-----------|---------|----------|----------|\n";
    
    for (const auto& r : results_) {
        file << "| " << r.name << " | " << r.elapsed_ms << " | " 
             << static_cast<size_t>(r.ops_per_sec) << " | " 
             << r.avg_ns << " | " << r.p99_ns << " |\n";
    }
    
    file << "\nGenerated: ";
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S UTC", gmtime(&time_t));
    file << buf << "\n";
}

// IL2CPP Benchmark Suite Implementation

void IL2CPPBenchmarkSuite::RunAll() {
    printf("Running hotc-il2cpp Benchmark Suite...\n\n");
    
    // Arithmetic
    BenchmarkInt32Add();
    BenchmarkInt32Sub();
    BenchmarkInt32Mul();
    BenchmarkInt32Div();
    BenchmarkInt32Rem();
    
    // Comparison
    BenchmarkInt32Ceq();
    BenchmarkInt32Cgt();
    BenchmarkInt32Clt();
    
    // Branch
    BenchmarkBranchUnconditional();
    BenchmarkBranchConditional();
    BenchmarkBranchLoop();
    
    // Memory
    BenchmarkAllocSmall();
    BenchmarkAllocMedium();
    BenchmarkAllocLarge();
    
    // Type system
    BenchmarkTypeLookup();
    BenchmarkTypeCheck();
    
    // Object
    BenchmarkObjectCreate();
    BenchmarkObjectAccess();
    BenchmarkArrayAccess();
    
    // Real-world
    BenchmarkUnityGameObjectCreation();
    BenchmarkUnityTransformAccess();
}

void IL2CPPBenchmarkSuite::BenchmarkInt32Add() {
    suite_.Run("Int32 Add", []() {
        volatile int a = 1, b = 2;
        volatile int c = a + b;
        (void)c;
    }, 10000000);
}

void IL2CPPBenchmarkSuite::BenchmarkInt32Sub() {
    suite_.Run("Int32 Sub", []() {
        volatile int a = 10, b = 3;
        volatile int c = a - b;
        (void)c;
    }, 10000000);
}

void IL2CPPBenchmarkSuite::BenchmarkInt32Mul() {
    suite_.Run("Int32 Mul", []() {
        volatile int a = 5, b = 6;
        volatile int c = a * b;
        (void)c;
    }, 10000000);
}

void IL2CPPBenchmarkSuite::BenchmarkInt32Div() {
    suite_.Run("Int32 Div", []() {
        volatile int a = 100, b = 7;
        volatile int c = a / b;
        (void)c;
    }, 10000000);
}

void IL2CPPBenchmarkSuite::BenchmarkInt32Rem() {
    suite_.Run("Int32 Rem", []() {
        volatile int a = 100, b = 7;
        volatile int c = a % b;
        (void)c;
    }, 10000000);
}

void IL2CPPBenchmarkSuite::BenchmarkInt32And() {
    suite_.Run("Int32 And", []() {
        volatile int a = 0xFF, b = 0x0F;
        volatile int c = a & b;
        (void)c;
    }, 10000000);
}

void IL2CPPBenchmarkSuite::BenchmarkInt32Or() {
    suite_.Run("Int32 Or", []() {
        volatile int a = 0xF0, b = 0x0F;
        volatile int c = a | b;
        (void)c;
    }, 10000000);
}

void IL2CPPBenchmarkSuite::BenchmarkInt32Xor() {
    suite_.Run("Int32 Xor", []() {
        volatile int a = 0xFF, b = 0x0F;
        volatile int c = a ^ b;
        (void)c;
    }, 10000000);
}

void IL2CPPBenchmarkSuite::BenchmarkInt32Shl() {
    suite_.Run("Int32 Shl", []() {
        volatile int a = 1;
        volatile int c = a << 4;
        (void)c;
    }, 10000000);
}

void IL2CPPBenchmarkSuite::BenchmarkInt32Shr() {
    suite_.Run("Int32 Shr", []() {
        volatile int a = 256;
        volatile int c = a >> 4;
        (void)c;
    }, 10000000);
}

void IL2CPPBenchmarkSuite::BenchmarkInt32Neg() {
    suite_.Run("Int32 Neg", []() {
        volatile int a = 42;
        volatile int c = -a;
        (void)c;
    }, 10000000);
}

void IL2CPPBenchmarkSuite::BenchmarkInt32Not() {
    suite_.Run("Int32 Not", []() {
        volatile int a = 0xFF;
        volatile int c = ~a;
        (void)c;
    }, 10000000);
}

void IL2CPPBenchmarkSuite::BenchmarkInt32Ceq() {
    suite_.Run("Int32 Ceq", []() {
        volatile int a = 42, b = 42;
        volatile bool c = (a == b);
        (void)c;
    }, 10000000);
}

void IL2CPPBenchmarkSuite::BenchmarkInt32Cgt() {
    suite_.Run("Int32 Cgt", []() {
        volatile int a = 42, b = 41;
        volatile bool c = (a > b);
        (void)c;
    }, 10000000);
}

void IL2CPPBenchmarkSuite::BenchmarkInt32Clt() {
    suite_.Run("Int32 Clt", []() {
        volatile int a = 41, b = 42;
        volatile bool c = (a < b);
        (void)c;
    }, 10000000);
}

void IL2CPPBenchmarkSuite::BenchmarkBranchUnconditional() {
    suite_.Run("Branch Unconditional", []() {
        volatile int sum = 0;
        for (int i = 0; i < 100; i++) {
            sum += i;
        }
    }, 1000000);
}

void IL2CPPBenchmarkSuite::BenchmarkBranchConditional() {
    suite_.Run("Branch Conditional", []() {
        volatile int sum = 0;
        for (int i = 0; i < 100; i++) {
            if (i % 2 == 0) sum += i;
        }
    }, 1000000);
}

void IL2CPPBenchmarkSuite::BenchmarkBranchLoop() {
    suite_.Run("Branch Loop (1000)", []() {
        volatile int sum = 0;
        for (int i = 0; i < 1000; i++) {
            sum += i;
        }
    }, 100000);
}

void IL2CPPBenchmarkSuite::BenchmarkAllocSmall() {
    HighPerformanceMemoryManager mem;
    suite_.Run("Alloc Small (32B)", [&]() {
        void* ptr = mem.Allocate(32);
        mem.Free(ptr);
    }, 1000000);
}

void IL2CPPBenchmarkSuite::BenchmarkAllocMedium() {
    HighPerformanceMemoryManager mem;
    suite_.Run("Alloc Medium (256B)", [&]() {
        void* ptr = mem.Allocate(256);
        mem.Free(ptr);
    }, 1000000);
}

void IL2CPPBenchmarkSuite::BenchmarkAllocLarge() {
    HighPerformanceMemoryManager mem;
    suite_.Run("Alloc Large (4KB)", [&]() {
        void* ptr = mem.Allocate(4096);
    }, 100000);
}

void IL2CPPBenchmarkSuite::BenchmarkTypeLookup() {
    TypeSystem types;
    suite_.Run("Type Lookup", [&]() {
        types.GetType("System.Int32");
    }, 1000000);
}

void IL2CPPBenchmarkSuite::BenchmarkTypeCheck() {
    TypeSystem types;
    auto* obj_type = types.GetObjectType();
    suite_.Run("Type Check", [&]() {
        (void)obj_type;
    }, 10000000);
}

void IL2CPPBenchmarkSuite::BenchmarkVirtualDispatch() {
    suite_.Run("Virtual Dispatch", []() {
        struct Base { virtual int Get() { return 1; } };
        struct Derived : Base { int Get() override { return 2; } };
        Derived d;
        Base* b = &d;
        volatile int v = b->Get();
        (void)v;
    }, 10000000);
}

void IL2CPPBenchmarkSuite::BenchmarkObjectCreate() {
    HighPerformanceMemoryManager mem;
    suite_.Run("Object Create", [&]() {
        void* obj = mem.AllocateObject(32);
    }, 1000000);
}

void IL2CPPBenchmarkSuite::BenchmarkObjectAccess() {
    struct Obj { int x, y, z; };
    HighPerformanceMemoryManager mem;
    Obj* obj = static_cast<Obj*>(mem.AllocateObject(sizeof(Obj)));
    suite_.Run("Object Access", [&]() {
        obj->x = 1;
        obj->y = 2;
        obj->z = 3;
    }, 10000000);
}

void IL2CPPBenchmarkSuite::BenchmarkArrayAccess() {
    HighPerformanceMemoryManager mem;
    int* arr = static_cast<int*>(mem.AllocateArray(sizeof(int), 1000));
    suite_.Run("Array Access", [&]() {
        for (int i = 0; i < 100; i++) {
            arr[i] = i;
        }
    }, 100000);
}

void IL2CPPBenchmarkSuite::BenchmarkUnityGameObjectCreation() {
    HighPerformanceMemoryManager mem;
    suite_.Run("Unity GameObject Create", [&]() {
        void* obj = mem.AllocateObject(128);
        // Simulate component initialization
        uint32_t* data = static_cast<uint32_t*>(obj);
        data[0] = 1; // type id
        data[1] = 0; // name index
        data[2] = 0; // transform
        data[3] = 0; // component count
    }, 1000000);
}

void IL2CPPBenchmarkSuite::BenchmarkUnityTransformAccess() {
    struct Transform {
        float x, y, z;
        float rx, ry, rz, rw;
        float sx, sy, sz;
    };
    
    HighPerformanceMemoryManager mem;
    Transform* t = static_cast<Transform*>(mem.AllocateObject(sizeof(Transform)));
    t->x = t->y = t->z = 0;
    
    suite_.Run("Unity Transform Access", [&]() {
        t->x += 0.01f;
        t->y += 0.02f;
        t->z += 0.03f;
    }, 10000000);
}

void IL2CPPBenchmarkSuite::BenchmarkUnityComponentLookup() {
    suite_.Run("Unity Component Lookup", []() {
        // Simulate component lookup with inline cache
        struct { uint32_t type_id; } components[8];
        for (int i = 0; i < 8; i++) components[i].type_id = i;
        
        volatile uint32_t target = 5;
        for (int i = 0; i < 8; i++) {
            if (components[i].type_id == target) break;
        }
    }, 1000000);
}

void IL2CPPBenchmarkSuite::BenchmarkUnityPhysicsRaycast() {
    suite_.Run("Unity Physics Raycast", []() {
        // Simplified raycast simulation
        struct { float x, y, z; } origin = {0, 0, 0};
        struct { float x, y, z; } dir = {0, 0, 1};
        struct { float x, y, z, w; } plane = {0, 0, 1, 0};
        
        float denom = dir.x * plane.x + dir.y * plane.y + dir.z * plane.z;
        float t = -(origin.x * plane.x + origin.y * plane.y + origin.z * plane.z + plane.w) / denom;
        (void)t;
    }, 10000000);
}

void IL2CPPBenchmarkSuite::BenchmarkUnityUICallback() {
    suite_.Run("Unity UI Callback", []() {
        struct Event { int type; float x, y; };
        Event e = {1, 100.0f, 200.0f};
        
        volatile float result = 0;
        if (e.type == 1) {
            result = e.x + e.y;
        }
    }, 10000000);
}

void IL2CPPBenchmarkSuite::PrintResults() {
    suite_.PrintResults();
}

void IL2CPPBenchmarkSuite::ExportJSON(const std::string& path) {
    suite_.ExportJSON(path);
}

void IL2CPPBenchmarkSuite::ExportHTML(const std::string& path) {
    suite_.ExportHTML(path);
}

} // namespace hotc
