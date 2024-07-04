#include "tst_common.h"

#include "threadpool.h"

#include <chrono>
#include <thread>

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
            //std::this_thread::sleep_for(1s);
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

class TreadPoolTest : public ::testing::Test {

};

TEST_F(TreadPoolTest, Multi_Task) {
    ThreadPool pool;

    ThreadObject obj;
    obj.setName("MainObject");
    for(int i = 0; i < 16; i++) {
        ThreadObject *object = new ThreadObject();
        object->setName(std::string("TestComponent") + std::to_string(i));
        object->setParent(&obj);
        object->post();
        pool.start(*object);
    }
    pool.waitForDone();

    for(auto it : obj.findChildren<ThreadObject *>()) {
        ASSERT_TRUE(it->counter() == uint32_t(1));
    }
}
