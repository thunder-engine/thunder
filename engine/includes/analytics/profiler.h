#ifndef PROFILER
#define PROFILER

#include <thread>
#include <unordered_map>

#include "engine.h"
#include "timer.h"

#ifdef PROFILING_ENABLED
    #define PROFILER_MARKER Profiler MARK(__FUNCTION__);

    #define PROFILER_STAT(x, y) Profiler::statAdd(x, y);
#else
    #define PROFILER_MARKER

    #define PROFILER_STAT(x, y)
#endif

class NEXT_LIBRARY_EXPORT Profiler {
public:
    struct CallPoint {
        const char             *name;

        TimePoint               started;

        TimePoint               stoped;
    };

public:
    __forceinline Profiler      (const char *name);

    __forceinline ~Profiler     ();

    static uint32_t             stat                (const char *name);

    static void                 statAdd             (const char *name, uint32_t value);

    static void                 statReset           (const char *name);

protected:
    Profiler::CallPoint         m_Current;

};

#endif // PROFILER

