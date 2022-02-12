#ifndef GLOBAL_H
#define GLOBAL_H

#if defined(SHARED_DEFINE) && defined(_WIN32)
    #ifdef NEXT_LIBRARY
        #define NEXT_LIBRARY_EXPORT __declspec(dllexport)
    #else
        #define NEXT_LIBRARY_EXPORT __declspec(dllimport)
    #endif
#else
    #define NEXT_LIBRARY_EXPORT
#endif

#ifdef PROFILING_ENABLED
    #ifdef BUILD_WITH_EASY_PROFILER
        #include <easy/profiler.h>

        #define PROFILE_BLOCK(name, ...) EASY_BLOCK(name, __VA_ARGS__)
        #define PROFILE_FUNCTION(...) EASY_FUNCTION(__VA_ARGS__)
        #define PROFILE_START EASY_PROFILER_ENABLE
        #define PROFILE_STOP profiler::dumpBlocksToFile("profile.prof")
        #define PROFILER_STAT(x, y)
        #define PROFILER_RESET(label)
    #else
        #include <analytics/profiler.h>

        #define PROFILE_BLOCK(name, ...)
        #define PROFILE_FUNCTION(...) Profiler MARK(__FUNCTION__);
        #define PROFILE_START
        #define PROFILE_STOP
        #define PROFILER_STAT(x, y)
        #define PROFILER_RESET(label)
    #endif
#else
    #define PROFILE_BLOCK(name, ...)
    #define PROFILE_FUNCTION(...)
    #define PROFILE_START
    #define PROFILE_STOP
    #define PROFILER_STAT(label, y)
    #define PROFILER_RESET(label)
#endif

#define A_UNUSED(a) (void)a

typedef float areal;

#endif // GLOBAL_H
