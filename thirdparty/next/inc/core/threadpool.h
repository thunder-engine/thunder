#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <stdint.h>

#include "object.h"

class ThreadPoolPrivate;

class NEXT_LIBRARY_EXPORT ThreadPool : public Object {
public:
    ThreadPool                  ();

    ~ThreadPool                 ();

    void                        start                       (Object &object);

    uint32_t                    maxThreads                  () const;

    void                        setMaxThreads               (uint32_t value);

    bool                        waitForDone                 (int32_t msecs = -1);

    static uint32_t             optimalThreadCount          ();

private:
    friend class ThreadPoolTest;

    ThreadPoolPrivate          *p_ptr;

};

#endif // ATHREADPOOL_H
