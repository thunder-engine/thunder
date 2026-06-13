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

#include "threadpool.h"

#include <thread>
#include <condition_variable>
#include <set>

class PoolWorker;

Runable::~Runable() {

}

bool Runable::autoDelete() const {
    return m_autoDelete;
}

void Runable::setAutoDelete(bool autoDelete) {
    m_autoDelete = autoDelete;
}

class ThreadPoolPrivate {
public:
    ThreadPoolPrivate() :
            m_activeThreads(0) {
        PROFILE_FUNCTION();
    }

    Runable *takeTask() {
        PROFILE_FUNCTION();
        if(!m_tasks.empty()) {
            Runable *runable = m_tasks.front();
            m_tasks.pop();
            return runable;
        }
        if(m_activeThreads > 0) {
            --m_activeThreads;
            m_variable.notify_one();
        }
        return nullptr;
    }

public:
    std::condition_variable m_variable;

    std::mutex m_mutex;

    std::set<PoolWorker *> m_workers;

    std::queue<Runable *> m_tasks;

    int32_t m_activeThreads;
};

class PoolWorker {
public:
    explicit PoolWorker(ThreadPoolPrivate *pool) :
            m_task(nullptr),
            m_pool(pool),
            m_enabled(true) {
        PROFILE_FUNCTION();

        m_thread = std::thread(&PoolWorker::exec, this);
    }
    ~PoolWorker() {
        PROFILE_FUNCTION();
        m_enabled = false;
        m_variable.notify_one();
        m_thread.join();
    }

    void exec() {
        PROFILE_FUNCTION();
        while(m_enabled) {
            std::unique_lock<std::mutex> locker(m_pool->m_mutex);
            m_variable.wait(locker, [&]() { return (m_task != nullptr) || !m_enabled; });

            if(m_task) {
                locker.unlock();
                m_task->run();
                if(m_task->autoDelete()) {
                    delete m_task;
                }
                locker.lock();
            }
            m_task = m_pool->takeTask();
            m_variable.notify_one();
        }
    }

    void run(Runable *runable) {
        PROFILE_FUNCTION();
        ++m_pool->m_activeThreads;
        m_task = runable;
        m_variable.notify_one();
    }

    bool isFree() {
        PROFILE_FUNCTION();
        return (m_task == nullptr);
    }

protected:
    std::thread m_thread;

    std::condition_variable m_variable;

    Runable *m_task;

    ThreadPoolPrivate *m_pool;

    bool m_enabled;
};
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
    Adds a \a runnable to run queue.
    In case of any free worker available executes task immediately.
*/
void ThreadPool::start(Runable *runnable) {
    PROFILE_FUNCTION();
    std::unique_lock<std::mutex> locker(p_ptr->m_mutex);
    for(auto it : p_ptr->m_workers) {
        if(it->isFree()) {
            it->run(runnable);
            return;
        }
    }
    p_ptr->m_tasks.push(runnable);
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
    uint32_t current = p_ptr->m_workers.size();
    if(current < number) {
        for(uint32_t i = 0; i < number - current; i++) {
            PoolWorker *worker = new PoolWorker(p_ptr);
            Runable *runable = p_ptr->takeTask();
            if(runable) {
                worker->run(runable);
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
    std::unique_lock<std::mutex> locker(p_ptr->m_mutex);
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
    return std::thread::hardware_concurrency();
}
