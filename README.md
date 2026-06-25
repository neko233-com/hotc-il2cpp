# hotc-il2cpp

High-performance IL2CPP interpreter for Unity, designed to surpass HybridCLR Pro.

## Features

- **Dual Interpreter Modes**: Classic stack-based + Token Threaded dispatch
- **Full IL2CPP Support**: Complete IL instruction set implementation
- **Unity CoreModule**: GameObject, Transform, Component bindings
- **IL-Level Testing**: Automated testing without Unity environment
- **Cross-Platform**: Windows, Linux, macOS support

## Architecture

```
┌─────────────────────────────────────────────────┐
│                  hotc-il2cpp                     │
├─────────────────────────────────────────────────┤
│  ┌──────────────┐  ┌──────────────┐            │
│  │   Classic     │  │   Token      │            │
│  │  Interpreter  │  │  Threaded    │            │
│  └──────┬───────┘  └──────┬───────┘            │
│         └────────┬────────┘                     │
│  ┌───────────────┴───────────────┐             │
│  │     IL Parser & Type System   │             │
│  └───────────────┬───────────────┘             │
│  ┌───────────────┴───────────────┐             │
│  │     Unity Bridge (C API)      │             │
│  └───────────────────────────────┘             │
└─────────────────────────────────────────────────┘
```

## Building

```bash
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

## Testing

```bash
cd build
ctest --output-on-failure
```

## Project Structure

```
hotc-il2cpp/
├── src/
│   ├── core/              # IL interpreter core
│   ├── bridge/            # Unity API bridge
│   └── utils/             # Utilities
├── tests/                 # IL-level tests
├── CMakeLists.txt
└── README.md
```

## Performance Goals

- **Classic Interpreter**: Baseline performance, easy to debug
- **Token Threaded**: 20-30% faster than classic
- **Target**: Surpass HybridCLR Pro on IL execution benchmarks

## License

Apache License 2.0
