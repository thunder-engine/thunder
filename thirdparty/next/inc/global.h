#ifndef AGLOBAL_H
#define AGLOBAL_H

#if defined(NEXT_SHARED) && defined(_WIN32)
    #ifdef NEXT_LIBRARY
        #define NEXT_LIBRARY_EXPORT __declspec(dllexport)
    #else
        #define NEXT_LIBRARY_EXPORT __declspec(dllimport)
    #endif
#else
    #define NEXT_LIBRARY_EXPORT
#endif

#ifdef BUILD_WITH_EASY_PROFILER
    #include <easy/profiler.h>

    #define PROFILE_BLOCK(name, ...) EASY_BLOCK(name, __VA_ARGS__)
    #define PROFILE_FUNCTION(...) EASY_FUNCTION(__VA_ARGS__)
    #define PROFILE_START EASY_PROFILER_ENABLE
    #define PROFILE_STOP profiler::dumpBlocksToFile("profile.prof");
#else
    #define PROFILE_BLOCK(name, ...)
    #define PROFILE_FUNCTION(...)
    #define PROFILE_START
    #define PROFILE_STOP
#endif

#define A_UNUSED(a) (void)a;

typedef float areal;

#endif // AGLOBAL_H
