#ifndef TIMER
#define TIMER

#include <chrono>

#include "engine.h"

typedef std::chrono::high_resolution_clock::time_point  TimePoint;

class NEXT_LIBRARY_EXPORT Timer {
public:
    static void                 init                        ();

    static void                 reset                       ();

    static void                 update                      ();

    static float                deltaTime                   ();

    static float                scale                       ();

    static void                 setScale                    (float scale);

    static float                time                        ();
};

#endif // TIMER

