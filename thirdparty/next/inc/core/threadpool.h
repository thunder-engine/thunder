/*
    This file is part of Thunder Next.

    Thunder Next is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

    Thunder Next is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Thunder Next.  If not, see <http://www.gnu.org/licenses/>.

    Copyright: 2008-2025 Evgeniy Prikazchikov
*/

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <stdint.h>

#include "object.h"

class ThreadPoolPrivate;

class NEXT_LIBRARY_EXPORT Runable {
public:
    virtual ~Runable();

    bool autoDelete() const;
    void setAutoDelete(bool autoDelete);

    virtual void run() = 0;
private:
    bool m_autoDelete = true;

};

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

#endif // THREADPOOL_H
