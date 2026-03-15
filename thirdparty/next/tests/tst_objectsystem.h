/*
    This file is part of Thunder Next.

    Thunder Next is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Thunder Next is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Thunder Next. If not, see <http://www.gnu.org/licenses/>.

    Copyright: 2008-2026 Evgeniy Prikazchikov
*/

#include "tst_common.h"

#include "objectsystem.h"

#include "json.h"
#include "bson.h"

class ObjectSecond : public TestObject {
    A_OBJECT(ObjectSecond, TestObject, Test)

    A_NOMETHODS()
    A_NOPROPERTIES()

};

class TestObjectEx : public TestObject {
    A_OBJECT_OVERRIDE(TestObjectEx, TestObject, Test)

    A_NOMETHODS()
    A_NOPROPERTIES()
};

namespace NextSuite {

    class ObjectSystemTest : public ::testing::Test {

    };

    TEST_F(ObjectSystemTest, RegisterUnregister_Object) {
        ObjectSystem objectSystem;

        ASSERT_TRUE((int)objectSystem.factories().size() == 0);
        ObjectSecond::registerClassFactory(&objectSystem);
        ASSERT_TRUE((int)objectSystem.factories().size() == 1);
        ObjectSecond::unregisterClassFactory(&objectSystem);
        ASSERT_TRUE((int)objectSystem.factories().size() == 0);
    }

    TEST_F(ObjectSystemTest, Object_Instansing) {
        ObjectSystem objectSystem;
        TestObject::registerClassFactory(&objectSystem);

        TestObject obj1;
        Object* result1 = ObjectSystem::objectCreate<TestObject>();
        Object* object = dynamic_cast<Object*>(&obj1);

        ASSERT_TRUE(result1 != nullptr);
        ASSERT_TRUE(object != nullptr);
        ASSERT_TRUE(compare(*object, *result1));

        Object* result2 = ObjectSystem::objectCreate<TestObject>();

        ASSERT_TRUE(result1->uuid() != result2->uuid());

        delete result1;
        delete result2;
    }

    TEST_F(ObjectSystemTest, Override_Object) {
        ObjectSystem objectSystem;
        TestObject::registerClassFactory(&objectSystem);

        TestObjectEx::registerClassFactory(&objectSystem);

        Object* object = ObjectSystem::objectCreate<TestObject>();

        ASSERT_TRUE(object != nullptr);
        const MetaObject* meta = object->metaObject();

        ASSERT_TRUE(dynamic_cast<TestObjectEx*>(object) != nullptr);
        ASSERT_TRUE(meta->methodCount() == 7);
        ASSERT_TRUE(meta->propertyCount() == 4);

        int index = meta->indexOfProperty("slot");
        ASSERT_TRUE(index > -1);
        delete object;

        TestObjectEx::unregisterClassFactory(&objectSystem);

        object = ObjectSystem::objectCreate<TestObject>();
        ASSERT_TRUE(dynamic_cast<TestObject*>(object) != nullptr);
        ASSERT_TRUE(dynamic_cast<TestObjectEx*>(object) == nullptr);
        delete object;
    }

    TEST_F(ObjectSystemTest, Serialize_Desirialize_Object) {
        ObjectSystem objectSystem;
        TestObject::registerClassFactory(&objectSystem);

        TestObject* obj1 = ObjectSystem::objectCreate<TestObject>();
        TestObject* obj2 = ObjectSystem::objectCreate<TestObject>();
        TestObject* obj3 = ObjectSystem::objectCreate<TestObject>();

        obj1->setName("MainObject");

        obj2->setName("TestComponent2");
        obj3->setName("TestComponent3");
        obj2->setParent(obj1);
        obj3->setParent(obj2);

        ASSERT_TRUE(Object::connect(obj1, _SIGNAL(signal(int)), obj2, _SLOT(setSlot(int))));
        ASSERT_TRUE(Object::connect(obj1, _SIGNAL(signal(int)), obj3, _SIGNAL(signal(int))));
        ASSERT_TRUE(Object::connect(obj2, _SIGNAL(signal(int)), obj3, _SLOT(setSlot(int))));

        ByteArray bytes = Bson::save(ObjectSystem::toVariant(obj1));
        Object* clone = obj1->clone();

        int id = obj1->uuid();
        int recv = obj1->getReceivers().size();

        delete obj3;
        delete obj2;
        delete obj1;

        Object* result = ObjectSystem::toObject(Bson::load(bytes));

        ASSERT_TRUE(result != nullptr);
        ASSERT_TRUE(compare(*clone, *result));

        ASSERT_TRUE(result->getReceivers().size() == recv);

        ASSERT_TRUE(result->uuid() == id);

        delete result;

        delete clone;
    }
}
