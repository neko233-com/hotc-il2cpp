#pragma once

// Platform detection
#if defined(_WIN32) || defined(_WIN64)
    #define HOTC_PLATFORM_WINDOWS 1
    #define HOTC_PLATFORM_NAME "Windows"
#elif defined(__APPLE__)
    #include <TargetConditionals.h>
    #if TARGET_OS_IOS
        #define HOTC_PLATFORM_IOS 1
        #define HOTC_PLATFORM_NAME "iOS"
    #elif TARGET_OS_SIMULATOR
        #define HOTC_PLATFORM_SIMULATOR 1
        #define HOTC_PLATFORM_NAME "iOS Simulator"
    #else
        #define HOTC_PLATFORM_MACOS 1
        #define HOTC_PLATFORM_NAME "macOS"
    #endif
#elif defined(__ANDROID__)
    #define HOTC_PLATFORM_ANDROID 1
    #define HOTC_PLATFORM_NAME "Android"
#elif defined(__EMSCRIPTEN__)
    #define HOTC_PLATFORM_WASM 1
    #define HOTC_PLATFORM_NAME "WebGL"
#elif defined(__linux__)
    #define HOTC_PLATFORM_LINUX 1
    #define HOTC_PLATFORM_NAME "Linux"
#else
    #define HOTC_PLATFORM_UNKNOWN 1
    #define HOTC_PLATFORM_NAME "Unknown"
#endif

// Compiler detection
#if defined(_MSC_VER)
    #define HOTC_COMPILER_MSVC 1
#elif defined(__GNUC__)
    #define HOTC_COMPILER_GCC 1
    #define HOTC_COMPILER_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#elif defined(__clang__)
    #define HOTC_COMPILER_CLANG 1
    #define HOTC_COMPILER_VERSION (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)
#endif

// Architecture detection
#if defined(__x86_64__) || defined(_M_X64)
    #define HOTC_ARCH_X64 1
#elif defined(__i386__) || defined(_M_IX86)
    #define HOTC_ARCH_X86 1
#elif defined(__aarch64__) || defined(_M_ARM64)
    #define HOTC_ARCH_ARM64 1
#elif defined(__arm__) || defined(_M_ARM)
    #define HOTC_ARCH_ARM 1
#elif defined(__EMSCRIPTEN__)
    #define HOTC_ARCH_WASM 1
#endif

// SIMD detection
#if defined(__SSE2__)
    #define HOTC_HAS_SSE2 1
#endif
#if defined(__SSE4_1__)
    #define HOTC_HAS_SSE4_1 1
#endif
#if defined(__AVX2__)
    #define HOTC_HAS_AVX2 1
#endif
#if defined(__ARM_NEON) || defined(__ARM_NEON__)
    #define HOTC_HAS_NEON 1
#endif

// Export macros
#if defined(_WIN32)
    #ifdef HOTC_EXPORTS
        #define HOTC_API __declspec(dllexport)
    #else
        #define HOTC_API __declspec(dllimport)
    #endif
#else
    #define HOTC_API __attribute__((visibility("default")))
#endif
