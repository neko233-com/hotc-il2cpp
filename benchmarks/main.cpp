#include "benchmark_suite.h"

int main() {
    printf("hotc-il2cpp Performance Benchmark Suite\n");
    printf("=======================================\n\n");
    
    hotc::IL2CPPBenchmarkSuite suite;
    suite.RunAll();
    suite.PrintResults();
    suite.ExportJSON("benchmarks/results.json");
    suite.ExportHTML("docs/benchmarks.html");
    suite.ExportMarkdown("benchmarks/RESULTS.md");
    
    printf("\nResults exported to:\n");
    printf("  - benchmarks/results.json\n");
    printf("  - docs/benchmarks.html\n");
    printf("  - benchmarks/RESULTS.md\n");
    
    return 0;
}
