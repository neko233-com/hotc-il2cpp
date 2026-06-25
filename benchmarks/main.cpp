#include "benchmark.h"
#include "../src/core/interpreter.h"
#include "../src/core/threaded.h"
#include "../src/core/type_system.h"
#include "../src/core/memory.h"
#include "../src/core/il_parser.h"

using namespace hotc;

void BenchmarkArithmetic() {
    TypeSystem types;
    MemoryManager memory;
    Interpreter interp(types, memory);

    MethodBody method;
    method.max_stack = 2;
    method.max_locals = 0;

    for (int i = 0; i < 100; i++) {
        method.instructions.push_back({OpCode::Ldc_I4, 1, 0});
        method.instructions.push_back({OpCode::Ldc_I4, 2, 1});
        method.instructions.push_back({OpCode::Add, 0, 2});
    }
    method.instructions.push_back({OpCode::Ret, 0, 100});

    Benchmark::Instance().Run("Classic Interpreter - Arithmetic", [&]() {
        interp.Execute(method);
    }, 100000);
}

void BenchmarkArithmeticThreaded() {
    TypeSystem types;
    MemoryManager memory;
    ThreadedInterpreter interp(types, memory);

    MethodBody method;
    method.max_stack = 2;
    method.max_locals = 0;

    for (int i = 0; i < 100; i++) {
        method.instructions.push_back({OpCode::Ldc_I4, 1, 0});
        method.instructions.push_back({OpCode::Ldc_I4, 2, 1});
        method.instructions.push_back({OpCode::Add, 0, 2});
    }
    method.instructions.push_back({OpCode::Ret, 0, 100});

    Benchmark::Instance().Run("Threaded Interpreter - Arithmetic", [&]() {
        interp.Execute(method);
    }, 100000);
}

void BenchmarkBranch() {
    TypeSystem types;
    MemoryManager memory;
    Interpreter interp(types, memory);

    MethodBody method;
    method.max_stack = 2;
    method.max_locals = 1;

    method.instructions.push_back({OpCode::Ldc_I4, 0, 0});
    method.instructions.push_back({OpCode::STloc_0, 0, 1});
    method.instructions.push_back({OpCode::LDloc_0, 0, 2});
    method.instructions.push_back({OpCode::Ldc_I4, 100, 3});
    method.instructions.push_back({OpCode::Blt, 0, 4});
    method.instructions.push_back({OpCode::LDloc_0, 0, 5});
    method.instructions.push_back({OpCode::Ldc_I4, 1, 6});
    method.instructions.push_back({OpCode::Add, 0, 7});
    method.instructions.push_back({OpCode::STloc_0, 0, 8});
    method.instructions.push_back({OpCode::Br, 2, 9});
    method.instructions.push_back({OpCode::Ret, 0, 10});

    Benchmark::Instance().Run("Classic Interpreter - Branch", [&]() {
        interp.Execute(method);
    }, 100000);
}

void BenchmarkBranchThreaded() {
    TypeSystem types;
    MemoryManager memory;
    ThreadedInterpreter interp(types, memory);

    MethodBody method;
    method.max_stack = 2;
    method.max_locals = 1;

    method.instructions.push_back({OpCode::Ldc_I4, 0, 0});
    method.instructions.push_back({OpCode::STloc_0, 0, 1});
    method.instructions.push_back({OpCode::LDloc_0, 0, 2});
    method.instructions.push_back({OpCode::Ldc_I4, 100, 3});
    method.instructions.push_back({OpCode::Blt, 0, 4});
    method.instructions.push_back({OpCode::LDloc_0, 0, 5});
    method.instructions.push_back({OpCode::Ldc_I4, 1, 6});
    method.instructions.push_back({OpCode::Add, 0, 7});
    method.instructions.push_back({OpCode::STloc_0, 0, 8});
    method.instructions.push_back({OpCode::Br, 2, 9});
    method.instructions.push_back({OpCode::Ret, 0, 10});

    Benchmark::Instance().Run("Threaded Interpreter - Branch", [&]() {
        interp.Execute(method);
    }, 100000);
}

void BenchmarkLoop() {
    TypeSystem types;
    MemoryManager memory;
    Interpreter interp(types, memory);

    MethodBody method;
    method.max_stack = 2;
    method.max_locals = 2;

    method.instructions.push_back({OpCode::Ldc_I4, 0, 0});
    method.instructions.push_back({OpCode::STloc_0, 0, 1});
    method.instructions.push_back({OpCode::Ldc_I4, 0, 2});
    method.instructions.push_back({OpCode::STloc_1, 0, 3});
    method.instructions.push_back({OpCode::LDloc_0, 0, 4});
    method.instructions.push_back({OpCode::Ldc_I4, 1000, 5});
    method.instructions.push_back({OpCode::Bge, 14, 6});
    method.instructions.push_back({OpCode::LDloc_1, 0, 7});
    method.instructions.push_back({OpCode::Ldc_I4, 1, 8});
    method.instructions.push_back({OpCode::Add, 0, 9});
    method.instructions.push_back({OpCode::STloc_1, 0, 10});
    method.instructions.push_back({OpCode::LDloc_0, 0, 11});
    method.instructions.push_back({OpCode::Ldc_I4, 1, 12});
    method.instructions.push_back({OpCode::Add, 0, 13});
    method.instructions.push_back({OpCode::STloc_0, 0, 14});
    method.instructions.push_back({OpCode::Br, 4, 15});
    method.instructions.push_back({OpCode::Ret, 0, 16});

    Benchmark::Instance().Run("Classic Interpreter - Loop (1000 iterations)", [&]() {
        interp.Execute(method);
    }, 10000);
}

void BenchmarkLoopThreaded() {
    TypeSystem types;
    MemoryManager memory;
    ThreadedInterpreter interp(types, memory);

    MethodBody method;
    method.max_stack = 2;
    method.max_locals = 2;

    method.instructions.push_back({OpCode::Ldc_I4, 0, 0});
    method.instructions.push_back({OpCode::STloc_0, 0, 1});
    method.instructions.push_back({OpCode::Ldc_I4, 0, 2});
    method.instructions.push_back({OpCode::STloc_1, 0, 3});
    method.instructions.push_back({OpCode::LDloc_0, 0, 4});
    method.instructions.push_back({OpCode::Ldc_I4, 1000, 5});
    method.instructions.push_back({OpCode::Bge, 14, 6});
    method.instructions.push_back({OpCode::LDloc_1, 0, 7});
    method.instructions.push_back({OpCode::Ldc_I4, 1, 8});
    method.instructions.push_back({OpCode::Add, 0, 9});
    method.instructions.push_back({OpCode::STloc_1, 0, 10});
    method.instructions.push_back({OpCode::LDloc_0, 0, 11});
    method.instructions.push_back({OpCode::Ldc_I4, 1, 12});
    method.instructions.push_back({OpCode::Add, 0, 13});
    method.instructions.push_back({OpCode::STloc_0, 0, 14});
    method.instructions.push_back({OpCode::Br, 4, 15});
    method.instructions.push_back({OpCode::Ret, 0, 16});

    Benchmark::Instance().Run("Threaded Interpreter - Loop (1000 iterations)", [&]() {
        interp.Execute(method);
    }, 10000);
}

void BenchmarkILParsing() {
    ILParser parser;
    uint8_t il_code[] = {0x00, 0x16, 0x17, 0x58, 0x2A};
    
    Benchmark::Instance().Run("IL Parser - Parse", [&]() {
        parser.Parse(il_code, sizeof(il_code));
    }, 1000000);
}

void BenchmarkTypeSystem() {
    TypeSystem types;
    
    Benchmark::Instance().Run("TypeSystem - GetType", [&]() {
        types.GetType("System.Int32");
    }, 1000000);
}

void BenchmarkMemoryAlloc() {
    MemoryManager memory;
    
    Benchmark::Instance().Run("MemoryManager - Allocate", [&]() {
        void* ptr = memory.Allocate(64);
        memory.Free(ptr);
    }, 1000000);
}

int main() {
    printf("hotc-il2cpp Performance Benchmarks\n");
    printf("==================================\n\n");

    BenchmarkArithmetic();
    BenchmarkArithmeticThreaded();
    BenchmarkBranch();
    BenchmarkBranchThreaded();
    BenchmarkLoop();
    BenchmarkLoopThreaded();
    BenchmarkILParsing();
    BenchmarkTypeSystem();
    BenchmarkMemoryAlloc();

    Benchmark::Instance().PrintResults();
    Benchmark::Instance().ExportJSON("benchmarks/results.json");
    Benchmark::Instance().ExportHTML("docs/site/benchmarks.html");

    printf("Results exported to benchmarks/results.json and docs/site/benchmarks.html\n");

    return 0;
}
