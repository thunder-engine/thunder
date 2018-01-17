#ifndef ATHREADPOOL_H
#define ATHREADPOOL_H

#include <stdint.h>

#include "aobject.h"

class AThreadPoolPrivate;

class NEXT_LIBRARY_EXPORT AThreadPool : public AObject {
public:
    AThreadPool                 ();

    ~AThreadPool                ();

    void                        start                       (AObject &object);

    uint32_t                    maxThreads                  () const;

    void                        setMaxThreads               (uint32_t value);

    bool                        waitForDone                 (int32_t msecs = -1);

    static uint32_t             optimalThreadCount          ();

private:
    friend class ThreadPoolTest;

    AThreadPoolPrivate         *p_ptr;

};

#endif // ATHREADPOOL_H
