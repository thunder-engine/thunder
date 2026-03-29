#include "os/backtrace.h"

#include "log.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")
#elif __APPLE__
#include <execinfo.h>
#include <cxxabi.h>
#include <unistd.h>
#include <signal.h>
#else
#include <execinfo.h>
#include <cxxabi.h>
#include <unistd.h>
#include <signal.h>
#endif

#define MAX_DEPTH 64

#ifdef _WIN32
LONG WINAPI exceptionHandler(EXCEPTION_POINTERS *exceptionInfo) {
    DWORD code = exceptionInfo->ExceptionRecord->ExceptionCode;
    const char *exceptionName;

    switch (code) {
        case EXCEPTION_ACCESS_VIOLATION: exceptionName = "Access Violation"; break;
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: exceptionName = "Array Bounds Exceeded"; break;
        case EXCEPTION_INT_DIVIDE_BY_ZERO: exceptionName = "Integer Division by Zero"; break;
        case EXCEPTION_STACK_OVERFLOW: exceptionName = "Stack Overflow"; break;
        default: exceptionName = "Unknown Exception"; break;
    }

    aCritical() << "====================================";
    aCritical() << "Program crashed with" << exceptionName << "(code " << (int)code << ")";
    aCritical() << "BackTrace:";
    int index = 0;
    for(auto &it : Backtrace::getBacktrace(3)) {
        aCritical() << index << ":" << it;
        ++index;
    }
    aCritical() << "====================================";

    return EXCEPTION_EXECUTE_HANDLER;
}
#else
void signalHandler(int sig) {
    const char *signalName;
    switch (sig) {
    case SIGSEGV: signalName = "SIGSEGV (Segmentation Fault)"; break;
    case SIGABRT: signalName = "SIGABRT (Abort)"; break;
    case SIGFPE:  signalName = "SIGFPE (Floating Point Exception)"; break;
    case SIGILL:  signalName = "SIGILL (Illegal Instruction)"; break;
    default:      signalName = "Unknown Signal"; break;
    }

    aCritical() << "====================================";
    aCritical() << "Program crashed with" << signalName << " (signal " << sig << ")";
    aCritical() << "BackTrace:";
    int index = 0;
    for(auto &it : Backtrace::getBacktrace(3)) {
        aCritical() << index << ":" << it;
        ++index;
    }
    aCritical() << "====================================";

    signal(sig, SIG_DFL);
    raise(sig);
}
#endif

void Backtrace::installCrashHandler() {
#ifdef _WIN32
    SetUnhandledExceptionFilter(exceptionHandler);
#else
    signal(SIGSEGV, signalHandler);
    signal(SIGABRT, signalHandler);
    signal(SIGFPE, signalHandler);
    signal(SIGILL, signalHandler);
#endif
}

#ifdef _WIN32
StringList Backtrace::getBacktrace(uint32_t skipFrames) {
    StringList result;

    static bool initialized = false;
    if (!initialized) {
        HANDLE process = GetCurrentProcess();
        SymInitialize(process, NULL, TRUE);
        initialized = true;
    }

    HANDLE process = GetCurrentProcess();
    void *stack[MAX_DEPTH];

    WORD frames = CaptureStackBackTrace(skipFrames, MAX_DEPTH, stack, NULL);

    for(WORD i = 0; i < frames; ++i) {
        SYMBOL_INFO* symbol = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
        symbol->MaxNameLen = 255;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

        DWORD64 address = (DWORD64)stack[i];

        if(SymFromAddr(process, address, 0, symbol)) {
            std::string frameStr = symbol->Name;

            // Add offset
            IMAGEHLP_MODULE64 module_info = { sizeof(IMAGEHLP_MODULE64) };
            if (SymGetModuleInfo64(process, address, &module_info)) {
                DWORD64 offset = address - module_info.BaseOfImage;
                frameStr += " + 0x" + std::to_string(offset);
            }

            result.push_back(frameStr);
        } else {
            result.push_back("[unknown]");
        }

        free(symbol);
    }

    return result;
}
#elif __APPLE__
StringList Backtrace::getBacktrace(uint32_t skipFrames) {
    StringList result;
    void *buffer[MAX_DEPTH];

    int frames = backtrace(buffer, MAX_DEPTH);
    char **symbols = backtrace_symbols(buffer, frames);

    if(!symbols) {
        return result;
    }

    for(int i = skipFrames; i < frames; ++i) {
        std::string frame = symbols[i];

        size_t start = frame.find(' ');
        if(start != std::string::npos) {
            size_t end = frame.find('+', start);
            if (end != std::string::npos) {
                std::string funcName = frame.substr(start + 1, end - start - 1);
                int status = 0;
                std::unique_ptr<char, void(*)(void*)> demangled(
                    abi::__cxa_demangle(funcName.c_str(), nullptr, nullptr, &status),
                    std::free
                    );
                if (status == 0 && demangled) {
                    frame.replace(start + 1, funcName.length(), demangled.get());
                }
            }
        }

        result.push_back(frame);
    }

    free(symbols);
    return result;
}
#else
StringList Backtrace::getBacktrace(uint32_t skipFrames) {
    StringList result;
    void *buffer[MAX_DEPTH];

    int frames = backtrace(buffer, MAX_DEPTH);
    char **symbols = backtrace_symbols(buffer, frames);

    if(!symbols) {
        return result;
    }

    for(int i = skipFrames; i < frames; ++i) {
        std::string frame;

        char *mangledName = nullptr;
        char *offsetBegin = nullptr;
        char *offsetEnd = nullptr;

        for(char *p = symbols[i]; *p; ++p) {
            if (*p == '(') {
                mangledName = p + 1;
            } else if (*p == '+') {
                offsetBegin = p;
            } else if (*p == ')') {
                offsetEnd = p;
                break;
            }
        }

        if(mangledName && offsetBegin && offsetEnd) {
            *offsetBegin = '\0';
            *offsetEnd = '\0';

            int status = 0;
            std::unique_ptr<char, void(*)(void*)> demangled(
                abi::__cxa_demangle(mangledName, nullptr, nullptr, &status),
                std::free
                );

            if(status == 0 && demangled) {
                frame = demangled.get();
            } else {
                frame = mangledName;
            }

            frame += offsetBegin;
        } else {
            frame = symbols[i];
        }

        result.push_back(frame);
    }

    free(symbols);
    return result;
}
#endif
