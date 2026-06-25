#include "benchmark.h"
#include <algorithm>
#include <fstream>
#include <sstream>

namespace hotc {

Benchmark& Benchmark::Instance() {
    static Benchmark instance;
    return instance;
}

void Benchmark::Run(const std::string& name, std::function<void()> func, size_t iterations) {
    // Warmup
    for (size_t i = 0; i < 1000; i++) {
        func();
    }

    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < iterations; i++) {
        func();
    }
    auto end = std::chrono::high_resolution_clock::now();

    double elapsed = std::chrono::duration<double, std::milli>(end - start).count();
    double ops_per_sec = (iterations / elapsed) * 1000.0;

    results_.push_back({name, elapsed, iterations, ops_per_sec});
}

void Benchmark::Run(const std::string& name, std::function<void(int)> func, size_t iterations) {
    // Warmup
    for (size_t i = 0; i < 1000; i++) {
        func(static_cast<int>(i));
    }

    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < iterations; i++) {
        func(static_cast<int>(i));
    }
    auto end = std::chrono::high_resolution_clock::now();

    double elapsed = std::chrono::duration<double, std::milli>(end - start).count();
    double ops_per_sec = (iterations / elapsed) * 1000.0;

    results_.push_back({name, elapsed, iterations, ops_per_sec});
}

void Benchmark::PrintResults() {
    printf("\n=== Benchmark Results ===\n");
    printf("%-40s %12s %12s %15s\n", "Name", "Time (ms)", "Iterations", "Ops/sec");
    printf("%-40s %12s %12s %15s\n", "----", "---------", "----------", "--------");
    
    for (const auto& result : results_) {
        printf("%-40s %12.2f %12zu %15.2f\n", 
            result.name.c_str(), 
            result.elapsed_ms, 
            result.iterations, 
            result.ops_per_sec);
    }
    printf("\n");
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
        file << "    \"ops_per_sec\": " << r.ops_per_sec << "\n";
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
        .container { max-width: 1200px; margin: 0 auto; padding: 2rem; }
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

    file << R"("</div>
            </div>
        </div>
        <table>
            <thead>
                <tr>
                    <th>Benchmark</th>
                    <th>Time (ms)</th>
                    <th>Iterations</th>
                    <th>Ops/sec</th>
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

} // namespace hotc
