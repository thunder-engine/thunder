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
            //std::this_thread::sleep_for(1s);
            m_counter++;
            return true;
        }
        return false;
    }

    void processEvents() {
        Object::processEvents();
    }

    uint32_t counter() const {
        return m_counter;
    }

    uint32_t m_counter;
};

namespace NextSuite {
    class TreadPoolTest : public ::testing::Test {

    };

    class ObjectRunner : public Runable {
    public:
        ObjectRunner(ThreadObject *object) :
            m_object(object) {

        }

        void run() {
            m_object->processEvents();
        }

        ThreadObject *m_object;
    };

    TEST_F(TreadPoolTest, Multi_Task) {
        ThreadPool pool;

        ThreadObject obj;
        obj.setName("MainObject");
        for (int i = 0; i < 16; i++) {
            ThreadObject *object = new ThreadObject();
            object->setName(std::string("TestComponent") + std::to_string(i));
            object->setParent(&obj);
            object->post();
            pool.start(new ObjectRunner(object));
        }
        pool.waitForDone();

        for (auto it : obj.findChildren<ThreadObject*>()) {
            ASSERT_TRUE(it->counter() == uint32_t(1));
        }
    }
}
