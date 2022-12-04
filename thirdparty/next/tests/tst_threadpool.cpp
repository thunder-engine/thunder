#include "tst_common.h"

#include "threadpool.h"

class ThreadObject : public Object {
public:
    explicit ThreadObject() :
        Object(),
        m_counter(0) {

    }

    void post() {
        Event *e = new Event(Event::UserType);
        postEvent(e);
    }

    bool event(Event *e) {
        if(e->type() == Event::UserType) {
            QTest::qSleep(1000);
            m_counter++;
            return true;
        }
        return false;
    }

    uint32_t counter() const {
        return m_counter;
    }

    uint32_t m_counter;
};

class TreadPoolTest : public QObject {
    Q_OBJECT
private slots:

void Multi_Task() {
    qDebug() << "Optimal number of threads =" << ThreadPool::optimalThreadCount();

    ThreadPool pool;

    ThreadObject obj;
    obj.setName("MainObject");
    for(int i = 0; i < 16; i++) {
        ThreadObject *object = new ThreadObject();
        object->setName(string("TestComponent") + to_string(i));
        object->setParent(&obj);
        object->post();
        pool.start(*object);
    }
    pool.waitForDone();

    for(auto it : obj.findChildren<ThreadObject *>()) {
        QCOMPARE(it->counter(), uint32_t(1));
    }
}

} REGISTER(ThreadPool)

#include "tst_threadpool.moc"
