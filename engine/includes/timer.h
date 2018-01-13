#ifndef TIMER
#define TIMER

#include <chrono>

#include "engine.h"

typedef std::chrono::high_resolution_clock::time_point  TimePoint;

class NEXT_LIBRARY_EXPORT Timer {
public:
    static void                 init                        (double fixed);

    static void                 update                      ();

    static double               deltaTime                   ();

    static double               fixedDelta                  ();

    static double               scale                       ();

    static void                 setScale                    (double scale);

    static double               time                        ();
};

#endif // TIMER

