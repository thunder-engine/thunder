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

    Copyright: 2008-2023 Evgeniy Prikazchikov
*/

#include "core/threadpool.h"

#include <thread>
#include <condition_variable>
#include <set>

class PoolWorker;

class ThreadPoolPrivate {
public:
    ThreadPoolPrivate() :
            m_activeThreads(0) {
        PROFILE_FUNCTION();
    }

    Object *takeTask() {
        PROFILE_FUNCTION();
        if(!m_tasks.empty()) {
            Object *object = m_tasks.front();
            m_tasks.pop();
            return object;
        }
        if(m_activeThreads > 0) {
            --m_activeThreads;
            m_variable.notify_one();
        }
        return nullptr;
    }

public:
    condition_variable m_variable;

    mutex m_mutex;

    set<PoolWorker *> m_workers;

    queue<Object *> m_tasks;

    int32_t m_activeThreads;
};

class PoolWorker {
public:
    explicit PoolWorker(ThreadPoolPrivate *pool);

    ~PoolWorker();

    void exec();

    void run(Object *object);

    bool isFree();

protected:
    bool m_enabled;

    thread m_thread;

    condition_variable m_variable;

    Object *m_task;

    ThreadPoolPrivate *m_pool;

};

PoolWorker::PoolWorker(ThreadPoolPrivate *pool) :
        m_enabled(true),
        m_task(nullptr),
        m_pool(pool) {
    PROFILE_FUNCTION();

    m_thread = thread(&PoolWorker::exec, this);
}

PoolWorker::~PoolWorker() {
    PROFILE_FUNCTION();
    m_enabled = false;
    m_variable.notify_one();
    m_thread.join();
}

void PoolWorker::exec() {
    PROFILE_FUNCTION();
    while(m_enabled) {
        unique_lock<mutex> locker(m_pool->m_mutex);
        m_variable.wait(locker, [&]() { return (m_task != nullptr) || !m_enabled; });

        if(m_task) {
            locker.unlock();
            m_task->processEvents();
            locker.lock();
        }
        m_task = m_pool->takeTask();
        m_variable.notify_one();
    }
}

void PoolWorker::run(Object *object) {
    PROFILE_FUNCTION();
    ++m_pool->m_activeThreads;
    m_task = object;
    m_variable.notify_one();
}

bool PoolWorker::isFree() {
    PROFILE_FUNCTION();
    return (m_task == nullptr);
}
/*!
    \class ThreadPool
    \brief The ThreadPool class manages a collection of threads.

    \since Next 1.0
    \inmodule Core
*/
ThreadPool::ThreadPool() :
        p_ptr(new ThreadPoolPrivate) {

    PROFILE_FUNCTION();
    setMaxThreads(optimalThreadCount());
}

ThreadPool::~ThreadPool() {
    PROFILE_FUNCTION();
    for(auto it : p_ptr->m_workers) {
        delete it;
    }
    p_ptr->m_workers.clear();
}
/*!
    Pushes an \a object to thread pool.
    In case of any free worker available executes task immediately.
*/
void ThreadPool::start(Object &object) {
    PROFILE_FUNCTION();
    unique_lock<mutex> locker(p_ptr->m_mutex);
    for(auto it : p_ptr->m_workers) {
        if(it->isFree()) {
            it->run(&object);
            return;
        }
    }
    p_ptr->m_tasks.push(&object);
}
/*!
    Returns the max number of threads allocated to work.
*/
uint32_t ThreadPool::maxThreads() const {
    PROFILE_FUNCTION();
    return p_ptr->m_workers.size();
}
/*!
    Sets the max \a number of threads allocated to work.
*/
void ThreadPool::setMaxThreads(uint32_t number) {
    PROFILE_FUNCTION();
    uint32_t current    = p_ptr->m_workers.size();
    if(current < number) {
        for(uint32_t i = 0; i < number - current; i++) {
            PoolWorker *worker = new PoolWorker(p_ptr);
            Object *object = p_ptr->takeTask();
            if(object) {
                worker->run(object);
            }
            p_ptr->m_workers.insert(worker);
        }
    } else if(current > number) {
        for(uint32_t i = 0; i < current - number; i++) {
            auto it = p_ptr->m_workers.end();
            --it;
            PoolWorker *worker = (*it);
            p_ptr->m_workers.erase(it);
            delete worker;
        }
    }
}
/*!
    Waits up to \a msecs milliseconds for all threads to exit and removes all threads from the thread pool.
    Returns true if all threads were removed; otherwise it returns false.
    If \a msecs is -1 (the default), the timeout is ignored (waits for the last thread to exit).
*/
bool ThreadPool::waitForDone(int32_t msecs) {
    PROFILE_FUNCTION();
    unique_lock<mutex> locker(p_ptr->m_mutex);
    if(msecs < 0) {
        while(true) {
            p_ptr->m_variable.wait(locker, [&]() { return (p_ptr->m_tasks.empty() && p_ptr->m_activeThreads == 0); });
            break;
        }
    }
    return (p_ptr->m_tasks.empty() && p_ptr->m_activeThreads == 0);
}
/*!
    Returns the optimal thread count for the current system.
    This value is based on the number of CPU cores.
*/
uint32_t ThreadPool::optimalThreadCount() {
    PROFILE_FUNCTION();
    return thread::hardware_concurrency();
}
