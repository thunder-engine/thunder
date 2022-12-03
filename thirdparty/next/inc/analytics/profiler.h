#ifndef PROFILER
#define PROFILER

#ifdef PROFILING_ENABLED

#include <thread>
#include <unordered_map>

#include "engine.h"
#include "timer.h"

class NEXT_LIBRARY_EXPORT Profiler {
public:
    struct CallPoint {
        const char *name;

        TimePoint started;

        TimePoint stoped;
    };

public:
    Profiler(const char *name);

    ~Profiler();

    static uint32_t stat(const char *name);

    static void statAdd(const char *name, uint32_t value);

    static void statReset(const char *name);

protected:
    Profiler::CallPoint  m_current;

};

#endif

#endif // PROFILER

