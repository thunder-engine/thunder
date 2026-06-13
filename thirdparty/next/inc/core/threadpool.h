/*
    This file is part of Thunder Next.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <stdint.h>

#include <object.h>

class ThreadPoolPrivate;
class Runable;

class NEXT_LIBRARY_EXPORT ThreadPool : public Object {
public:
    ThreadPool();

    ~ThreadPool();

    void start(Runable *runnable);

    uint32_t maxThreads() const;

    void setMaxThreads(uint32_t value);

    bool waitForDone(int32_t msecs = -1);

    static uint32_t optimalThreadCount();

private:
    friend class ThreadPoolTest;

    ThreadPoolPrivate *p_ptr;

};

class NEXT_LIBRARY_EXPORT Runable {
public:
    virtual ~Runable();

    bool autoDelete() const;
    void setAutoDelete(bool autoDelete);

    virtual void run() = 0;
private:
    bool m_autoDelete = true;

};

#endif // THREADPOOL_H
