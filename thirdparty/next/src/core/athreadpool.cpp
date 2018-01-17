#include "core/athreadpool.h"

#include <thread>
#include <condition_variable>
#include <set>

class AThreadPoolPrivate {
public:
    AThreadPoolPrivate() :
            m_ActiveThreads(0) {
        PROFILE_FUNCTION()
    }

    AObject *takeTask() {
        PROFILE_FUNCTION()
        if(!m_Tasks.empty()) {
            AObject *object = m_Tasks.front();
            m_Tasks.pop();
            return object;
        }
        if(m_ActiveThreads > 0) {
            --m_ActiveThreads;
            m_Variable.notify_one();
        }
        return nullptr;
    }

    class APoolWorker {
    public:
        APoolWorker             (AThreadPoolPrivate *pool);

        ~APoolWorker            ();

        void                    exec                        ();

        void                    run                         (AObject *object);

        bool                    isFree                      ();

    protected:
        bool                    m_Enabled;

        thread                  m_Thread;

        condition_variable      m_Variable;

        AObject                *m_pTask;

        AThreadPoolPrivate     *m_pPool;
    };

public:
    condition_variable          m_Variable;

    mutex                       m_Mutex;

    set<APoolWorker *>          m_Workers;

    queue<AObject *>            m_Tasks;

    int32_t                     m_ActiveThreads;
};

AThreadPoolPrivate::APoolWorker::APoolWorker(AThreadPoolPrivate *pool) :
        m_pPool(pool),
        m_pTask(nullptr),
        m_Enabled(true) {
    PROFILE_FUNCTION()
    m_Thread    = thread(&APoolWorker::exec, this);
}

AThreadPoolPrivate::APoolWorker::~APoolWorker() {
    PROFILE_FUNCTION()
    m_Enabled   = false;
    m_Variable.notify_one();
    m_Thread.join();
}

void AThreadPoolPrivate::APoolWorker::exec() {
    PROFILE_FUNCTION()
    while(m_Enabled) {
        unique_lock<mutex> locker(m_pPool->m_Mutex);
        m_Variable.wait(locker, [&]() { return (m_pTask != nullptr) || !m_Enabled; });

        if(m_pTask) {
            locker.unlock();
            m_pTask->processEvents();
            locker.lock();
        }
        m_pTask = m_pPool->takeTask();
        m_Variable.notify_one();
    }
}

void AThreadPoolPrivate::APoolWorker::run(AObject *object) {
    PROFILE_FUNCTION()
    ++m_pPool->m_ActiveThreads;
    m_pTask = object;
    m_Variable.notify_one();
}

bool AThreadPoolPrivate::APoolWorker::isFree() {
    PROFILE_FUNCTION()
    return (m_pTask == nullptr);
}

AThreadPool::AThreadPool() :
        p_ptr(new AThreadPoolPrivate) {

    PROFILE_FUNCTION()
    setMaxThreads(optimalThreadCount());
}

AThreadPool::~AThreadPool() {
    PROFILE_FUNCTION()
    for(auto it : p_ptr->m_Workers) {
        delete it;
    }
    p_ptr->m_Workers.clear();
}

void AThreadPool::start(AObject &object) {
    PROFILE_FUNCTION()
    unique_lock<mutex> locker(p_ptr->m_Mutex);
    for(auto it : p_ptr->m_Workers) {
        if(it->isFree()) {
            it->run(&object);
            return;
        }
    }
    p_ptr->m_Tasks.push(&object);
}

uint32_t AThreadPool::maxThreads() const {
    PROFILE_FUNCTION()
    return p_ptr->m_Workers.size();
}

void AThreadPool::setMaxThreads(uint32_t value) {
    PROFILE_FUNCTION()
    uint32_t current    = p_ptr->m_Workers.size();
    if(current < value) {
        for(uint32_t i = 0; i < value - current; i++) {
            AThreadPoolPrivate::APoolWorker *worker = new AThreadPoolPrivate::APoolWorker(p_ptr);
            AObject *object     = p_ptr->takeTask();
            if(object) {
                worker->run(object);
            }
            p_ptr->m_Workers.insert(worker);
        }
    } else if(current > value) {
        for(uint32_t i = 0; i < current - value; i++) {
            auto it = p_ptr->m_Workers.end();
            --it;
            AThreadPoolPrivate::APoolWorker *worker = (*it);
            p_ptr->m_Workers.erase(it);
            delete worker;
        }
    }
}

bool AThreadPool::waitForDone(int32_t msecs) {
    PROFILE_FUNCTION()
    unique_lock<mutex> locker(p_ptr->m_Mutex);
    if(msecs < 0) {
        while(true) {
            p_ptr->m_Variable.wait(locker, [&]() { return (p_ptr->m_Tasks.empty() && p_ptr->m_ActiveThreads == 0); });
            break;
        }
    }
    return (p_ptr->m_Tasks.empty() && p_ptr->m_ActiveThreads == 0);
}

uint32_t AThreadPool::optimalThreadCount() {
    PROFILE_FUNCTION()
    return thread::hardware_concurrency();
}
