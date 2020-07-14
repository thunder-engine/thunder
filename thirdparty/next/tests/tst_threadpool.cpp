#include "tst_common.h"

#include "threadpool.h"

class ThreadObject : public Object {
public:
    explicit ThreadObject     () :
            Object() {
        m_Counter   = 0;
    }

    void            post            () {
        Event *e = new Event(Event::UserType);
        postEvent(e);
    }

    bool            event           (Event *e) {
        if(e->type() == Event::UserType) {
            QTest::qSleep(1000);
            m_Counter++;
            return true;
        }
        return false;
    }

    uint32_t        counter         () const {
        return m_Counter;
    }

    uint32_t        m_Counter;
};

class TreadPoolTest : public QObject {
    Q_OBJECT

    ThreadPool *m_pPool;

private slots:

void initTestCase() {
    qDebug() << "Optimal number of threads =" << ThreadPool::optimalThreadCount();
    m_pPool = new ThreadPool;
}

void cleanupTestCase() {
    delete m_pPool;
}

void Multi_Task() {
    ThreadObject obj;
    obj.setName("MainObject");
    for(int i = 0; i < 16; i++) {
        ThreadObject *object = new ThreadObject();
        object->setName(string("TestComponent") + to_string(i));
        object->setParent(&obj);
        object->post();
        m_pPool->start(*object);
    }
    m_pPool->waitForDone();

    for(auto it : obj.findChildren<ThreadObject *>()) {
        QCOMPARE(it->counter(), uint32_t(1));
    }
}

} REGISTER(ThreadPool)

#include "tst_threadpool.moc"
