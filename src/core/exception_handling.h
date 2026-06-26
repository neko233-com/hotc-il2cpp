#pragma once

#include "platform.h"
#include <cstdint>
#include <vector>
#include <functional>
#include <exception>
#include <string>

namespace hotc {

// Exception types
enum class ExceptionType {
    None,
    NullReference,
    InvalidCast,
    Overflow,
    DivideByZero,
    IndexOutOfRange,
    ArgumentNull,
    ArgumentOutOfRange,
    InvalidOperationException,
    NotSupportedException,
    OutOfMemory,
    StackOverflow,
    ThreadAbort,
    AppDomainUnloaded,
    AmbiguousMatch,
    TypeLoad,
    MissingMember,
    RuntimeWrapped
};

// IL2CPP Exception object layout
struct Il2CppObject {
    void* klass;
    uint32_t monitor;
    uint32_t reserved;
};

struct Il2CppException : Il2CppObject {
    Il2CppObject* inner_ex;
    const char* message;
    const char* help_link;
    const char* class_name;
    const char* source;
    int32_t hresult;
    void* trace_ips;
    void* dynamic_methods;
    int32_t remote_stack_index;
    void* remote_source;
    void* data;
    void* caught_in_unmanaged;
};

// Exception handling frame
struct ExceptionFrame {
    uint32_t try_start;
    uint32_t try_end;
    uint32_t handler_start;
    uint32_t handler_end;
    uint32_t filter_start;
    ExceptionType catch_type;
    uint32_t catch_type_index;
    uint32_t stack_depth;
    void* exception;
    ExceptionFrame* prev;
    bool is_finally;
};

// Exception handler for IL2CPP
class ExceptionHandler {
public:
    ExceptionHandler() = default;
    ~ExceptionHandler() = default;
    
    // Push/pop exception frames
    void PushFrame(uint32_t try_start, uint32_t try_end, uint32_t handler_start, 
                   uint32_t handler_end, uint32_t filter_start, 
                   ExceptionType catch_type, uint32_t catch_type_index);
    void PopFrame();
    
    // Find handler for exception
    ExceptionFrame* FindHandler(uint32_t pc, ExceptionType exc_type, uint32_t exc_type_index) const;
    ExceptionFrame* FindFinallyHandler(uint32_t pc) const;
    
    // Create exception objects
    static Il2CppException* CreateNullReference();
    static Il2CppException* CreateInvalidCast();
    static Il2CppException* CreateOverflow();
    static Il2CppException* CreateDivideByZero();
    static Il2CppException* CreateIndexOutOfRange(int32_t index, int32_t length);
    static Il2CppException* CreateArgumentNull(const char* param);
    static Il2CppException* CreateArgumentOutOfRange(const char* param);
    static Il2CppException* CreateInvalidOperation(const char* msg);
    static Il2CppException* CreateNotSupported(const char* msg);
    static Il2CppException* CreateOutOfMemory();
    static Il2CppException* CreateTypeLoad(const char* type_name);
    static Il2CppException* CreateMissingMember(const char* member_name);
    static Il2CppException* CreateRuntime(const char* message);
    
    // Exception properties
    bool HasException() const { return current_exception_ != nullptr; }
    void* GetCurrentException() const { return current_exception_; }
    void SetCurrentException(void* exc) { current_exception_ = exc; }
    void ClearCurrentException() { current_exception_ = nullptr; }
    
    // Stack trace
    std::string GetStackTrace() const;
    
    // Exception propagation
    void Throw(void* exception);
    void Rethrow();
    void ThrowExplicit(void* exception);
    
private:
    ExceptionFrame* frame_stack_ = nullptr;
    void* current_exception_ = nullptr;
};

// Exception filter for when clauses
struct ExceptionFilter {
    uint32_t filter_start;
    uint32_t filter_end;
    uint32_t catch_type_index;
};

// Try-catch-finally block info
struct TryCatchBlock {
    uint32_t try_start;
    uint32_t try_end;
    uint32_t handler_start;
    uint32_t handler_end;
    uint32_t filter_start;
    ExceptionType catch_type;
    uint32_t catch_type_index;
    bool has_finally;
    uint32_t finally_start;
    uint32_t finally_end;
};

} // namespace hotc
