#include "exception_handling.h"
#include <cstring>
#include <sstream>

namespace hotc {

void ExceptionHandler::PushFrame(uint32_t try_start, uint32_t try_end, uint32_t handler_start,
                                  uint32_t handler_end, uint32_t filter_start,
                                  ExceptionType catch_type, uint32_t catch_type_index) {
    ExceptionFrame* frame = new ExceptionFrame();
    frame->try_start = try_start;
    frame->try_end = try_end;
    frame->handler_start = handler_start;
    frame->handler_end = handler_end;
    frame->filter_start = filter_start;
    frame->catch_type = catch_type;
    frame->catch_type_index = catch_type_index;
    frame->exception = nullptr;
    frame->prev = frame_stack_;
    frame->is_finally = false;
    frame_stack_ = frame;
}

void ExceptionHandler::PopFrame() {
    if (frame_stack_) {
        ExceptionFrame* frame = frame_stack_;
        frame_stack_ = frame->prev;
        delete frame;
    }
}

ExceptionFrame* ExceptionHandler::FindHandler(uint32_t pc, ExceptionType exc_type, uint32_t exc_type_index) const {
    ExceptionFrame* frame = frame_stack_;
    while (frame) {
        if (pc >= frame->try_start && pc < frame->try_end) {
            if (frame->catch_type == ExceptionType::None || frame->catch_type == exc_type) {
                if (frame->catch_type_index == 0 || frame->catch_type_index == exc_type_index) {
                    return frame;
                }
            }
        }
        frame = frame->prev;
    }
    return nullptr;
}

ExceptionFrame* ExceptionHandler::FindFinallyHandler(uint32_t pc) const {
    ExceptionFrame* frame = frame_stack_;
    while (frame) {
        if (pc >= frame->try_start && pc < frame->try_end) {
            return frame;
        }
        frame = frame->prev;
    }
    return nullptr;
}

Il2CppException* ExceptionHandler::CreateNullReference() {
    auto* exc = new Il2CppException();
    exc->klass = nullptr;
    exc->inner_ex = nullptr;
    exc->message = "Object reference not set to an instance of an object";
    exc->help_link = nullptr;
    exc->class_name = "System.NullReferenceException";
    exc->source = nullptr;
    exc->hresult = -2147467261;
    return exc;
}

Il2CppException* ExceptionHandler::CreateInvalidCast() {
    auto* exc = new Il2CppException();
    exc->klass = nullptr;
    exc->inner_ex = nullptr;
    exc->message = "Unable to cast object to its derived type";
    exc->help_link = nullptr;
    exc->class_name = "System.InvalidCastException";
    exc->source = nullptr;
    exc->hresult = -2146233088;
    return exc;
}

Il2CppException* ExceptionHandler::CreateOverflow() {
    auto* exc = new Il2CppException();
    exc->klass = nullptr;
    exc->inner_ex = nullptr;
    exc->message = "Arithmetic operation resulted in an overflow";
    exc->help_link = nullptr;
    exc->class_name = "System.OverflowException";
    exc->source = nullptr;
    exc->hresult = -2146233066;
    return exc;
}

Il2CppException* ExceptionHandler::CreateDivideByZero() {
    auto* exc = new Il2CppException();
    exc->klass = nullptr;
    exc->inner_ex = nullptr;
    exc->message = "Attempted to divide by zero";
    exc->help_link = nullptr;
    exc->class_name = "System.DivideByZeroException";
    exc->source = nullptr;
    exc->hresult = -2146233067;
    return exc;
}

Il2CppException* ExceptionHandler::CreateIndexOutOfRange(int32_t index, int32_t length) {
    auto* exc = new Il2CppException();
    exc->klass = nullptr;
    exc->inner_ex = nullptr;
    std::string msg = "Index was outside the bounds of the array";
    exc->message = msg.c_str();
    exc->help_link = nullptr;
    exc->class_name = "System.IndexOutOfRangeException";
    exc->source = nullptr;
    exc->hresult = -2146233080;
    return exc;
}

Il2CppException* ExceptionHandler::CreateArgumentNull(const char* param) {
    auto* exc = new Il2CppException();
    exc->klass = nullptr;
    exc->inner_ex = nullptr;
    exc->message = "Value cannot be null";
    exc->help_link = nullptr;
    exc->class_name = "System.ArgumentNullException";
    exc->source = nullptr;
    exc->hresult = -2147467261;
    return exc;
}

Il2CppException* ExceptionHandler::CreateArgumentOutOfRange(const char* param) {
    auto* exc = new Il2CppException();
    exc->klass = nullptr;
    exc->inner_ex = nullptr;
    exc->message = "Specified argument was out of the range of valid values";
    exc->help_link = nullptr;
    exc->class_name = "System.ArgumentOutOfRangeException";
    exc->source = nullptr;
    exc->hresult = -2146233086;
    return exc;
}

Il2CppException* ExceptionHandler::CreateInvalidOperation(const char* msg) {
    auto* exc = new Il2CppException();
    exc->klass = nullptr;
    exc->inner_ex = nullptr;
    exc->message = msg;
    exc->help_link = nullptr;
    exc->class_name = "System.InvalidOperationException";
    exc->source = nullptr;
    exc->hresult = -2146233079;
    return exc;
}

Il2CppException* ExceptionHandler::CreateNotSupported(const char* msg) {
    auto* exc = new Il2CppException();
    exc->klass = nullptr;
    exc->inner_ex = nullptr;
    exc->message = msg;
    exc->help_link = nullptr;
    exc->class_name = "System.NotSupportedException";
    exc->source = nullptr;
    exc->hresult = -2146233067;
    return exc;
}

Il2CppException* ExceptionHandler::CreateOutOfMemory() {
    auto* exc = new Il2CppException();
    exc->klass = nullptr;
    exc->inner_ex = nullptr;
    exc->message = "Out of memory";
    exc->help_link = nullptr;
    exc->class_name = "System.OutOfMemoryException";
    exc->source = nullptr;
    exc->hresult = -2147024882;
    return exc;
}

Il2CppException* ExceptionHandler::CreateTypeLoad(const char* type_name) {
    auto* exc = new Il2CppException();
    exc->klass = nullptr;
    exc->inner_ex = nullptr;
    exc->message = "Could not load type";
    exc->help_link = nullptr;
    exc->class_name = "System.TypeLoadException";
    exc->source = nullptr;
    exc->hresult = -2146233054;
    return exc;
}

Il2CppException* ExceptionHandler::CreateMissingMember(const char* member_name) {
    auto* exc = new Il2CppException();
    exc->klass = nullptr;
    exc->inner_ex = nullptr;
    exc->message = "Member not found";
    exc->help_link = nullptr;
    exc->class_name = "System.MissingMemberException";
    exc->source = nullptr;
    exc->hresult = -2146233070;
    return exc;
}

Il2CppException* ExceptionHandler::CreateRuntime(const char* message) {
    auto* exc = new Il2CppException();
    exc->klass = nullptr;
    exc->inner_ex = nullptr;
    exc->message = message;
    exc->help_link = nullptr;
    exc->class_name = "System.Exception";
    exc->source = nullptr;
    exc->hresult = -2146232832;
    return exc;
}

std::string ExceptionHandler::GetStackTrace() const {
    std::ostringstream oss;
    ExceptionFrame* frame = frame_stack_;
    int depth = 0;
    while (frame) {
        oss << "  at frame " << depth << " (try: " << frame->try_start << "-" << frame->try_end 
            << ", handler: " << frame->handler_start << ")" << std::endl;
        frame = frame->prev;
        depth++;
    }
    return oss.str();
}

void ExceptionHandler::Throw(void* exception) {
    current_exception_ = exception;
    // Find matching handler
    ExceptionFrame* handler = FindHandler(0, ExceptionType::None, 0);
    if (handler) {
        // Jump to handler
    }
}

void ExceptionHandler::Rethrow() {
    if (current_exception_) {
        Throw(current_exception_);
    }
}

void ExceptionHandler::ThrowExplicit(void* exception) {
    current_exception_ = exception;
}

} // namespace hotc
