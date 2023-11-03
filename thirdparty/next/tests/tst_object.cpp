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
    along with Thunder Next.  If not, see <http://www.gnu.org/licenses/>.

    Copyright: 2008-2014 Evgeniy Prikazchikov
*/

#include "tst_common.h"

class ObjectTest : public QObject {
    Q_OBJECT
private slots:

void Disconnect_base() {
    TestObject obj1;
    TestObject obj2;
    TestObject obj3;

    Object::connect(&obj1, _SIGNAL(signal(int)), &obj2, _SLOT(setSlot(int)));
    Object::connect(&obj1, _SIGNAL(signal(int)), &obj3, _SIGNAL(signal(int)));
    QCOMPARE((int)obj1.getReceivers().size(), 2);

    Object::disconnect(&obj1, _SIGNAL(signal(int)), &obj3, _SIGNAL(signal(int)));
    QCOMPARE((int)obj1.getReceivers().size(), 1);
}

void Disconnect_all() {
    TestObject obj1;
    TestObject obj2;
    TestObject obj3;

    Object::connect(&obj1, _SIGNAL(signal(int)), &obj2, _SLOT(setSlot(int)));
    Object::connect(&obj1, _SIGNAL(signal(int)), &obj3, _SIGNAL(signal(int)));
    QCOMPARE((int)obj1.getReceivers().size(), 2);

    Object::disconnect(&obj1, 0, 0, 0);
    QCOMPARE((int)obj1.getReceivers().size(), 0);
}

void Disconnect_by_signal() {
    TestObject obj1;
    TestObject obj2;
    TestObject obj3;

    Object::connect(&obj1, _SIGNAL(signal(int)), &obj2, _SLOT(setSlot(int)));
    Object::connect(&obj1, _SIGNAL(signal(int)), &obj3, _SIGNAL(signal(int)));
    QCOMPARE((int)obj1.getReceivers().size(), 2);

    Object::disconnect(&obj1, _SIGNAL(signal(int)), 0, 0);
    QCOMPARE((int)obj1.getReceivers().size(), 0);
}

void Disconnect_by_receiver() {
    TestObject obj1;
    TestObject obj2;
    TestObject obj3;

    Object::connect(&obj1, _SIGNAL(signal(int)), &obj2, _SLOT(setSlot(int)));
    Object::connect(&obj1, _SIGNAL(signal(int)), &obj3, _SIGNAL(signal(int)));
    QCOMPARE((int)obj1.getReceivers().size(), 2);

    Object::disconnect(&obj1, 0, &obj3, 0);
    QCOMPARE((int)obj1.getReceivers().size(), 1);
}

void Child_destructor() {
    TestObject *obj1 = new TestObject;
    TestObject *obj2 = new TestObject();
    obj2->setName("TestComponent2");
    obj2->setParent(obj1);

    TestObject *obj3 = new TestObject();
    obj3->setName("TestComponent3");
    obj3->setParent(obj1);

    Object::connect(obj2, _SIGNAL(destroyed()), obj3, _SLOT(onDestroyed()));

    QCOMPARE((int)obj1->getChildren().size(), 2);

    delete obj2;
    QCOMPARE((int)obj1->getChildren().size(), 1);

    obj3->processEvents();
    QCOMPARE(obj3->getSlot(), true);

    obj3->deleteLater();
    obj3->processEvents();
    QCOMPARE((int)obj1->getChildren().size(), 0);

    delete obj1;
}

void Reciever_destructor() {
    TestObject *obj1 = new TestObject;
    TestObject *obj2 = new TestObject;

    Object::connect(obj1, _SIGNAL(signal(int)), obj2, _SIGNAL(signal(int)));
    QCOMPARE((int)obj1->getReceivers().size(),  1);

    delete obj2;
    QCOMPARE((int)obj1->getReceivers().size(),  0);

    delete obj1;
}

void Emit_signal() {
    TestObject obj1;
    TestObject obj2;
    TestObject obj3;

    Object::connect(&obj1, _SIGNAL(signal(int)), &obj2, _SIGNAL(signal(int)));
    Object::connect(&obj2, _SIGNAL(signal(int)), &obj3, _SLOT(setSlot(int)));

    {
        QCOMPARE(obj3.m_bSlot, 0);
        obj2.emitSignal(_SIGNAL(signal(int)), 1);

        obj1.processEvents();
        obj2.processEvents();
        obj3.processEvents();

        QCOMPARE(obj3.m_bSlot, 1);
    }
    {
        QCOMPARE(obj3.m_bSlot, 1);
        obj1.emitSignal(_SIGNAL(signal(int)), 0);

        obj1.processEvents();
        obj2.processEvents();
        obj3.processEvents();

        QCOMPARE(obj3.m_bSlot, 0);
    }
}

void Find_object() {
    Object obj1;
    TestObject obj2;
    TestObject obj3;

    obj1.setName("MainObject");
    obj2.setName("TestComponent2");
    obj3.setName("TestComponent3");
    obj2.setParent(&obj1);
    obj3.setParent(&obj1);
    {
        Object *result  = obj1.find("TestComponent2");
        QCOMPARE(result, &obj2);
    }
    {
        Object *result  = obj1.find("/MainObject/TestComponent3");
        QCOMPARE(&obj3, result);
    }
    {
        TestObject *result  = obj1.findChild<TestObject *>();
        QCOMPARE(&obj2, result);
    }
    {
        list<TestObject *> result   = obj1.findChildren<TestObject *>();
        QCOMPARE(int(result.size()), 2);
    }
}

void Clone_object() {
    ObjectSystem objectSystem;
    TestObject::registerClassFactory(&objectSystem);

    TestObject *obj1 = ObjectSystem::objectCreate<TestObject>();
    TestObject *obj2 = ObjectSystem::objectCreate<TestObject>();

    obj1->setName("MainObject");
    obj2->setName("TestComponent2");
    obj2->setParent(obj1);
    obj1->setVector(Vector2(10.0, 20.0));

    Object::connect(obj1, _SIGNAL(signal(int)), obj2, _SLOT(setSlot(int)));
    Object::connect(obj2, _SIGNAL(signal(int)), obj1, _SLOT(setSlot(int)));

    Object *clone = obj1->clone();
    QCOMPARE(compare(*clone, *obj1), true);
    QCOMPARE((clone->uuid() != 0), true);
    delete clone;

    delete obj2;
    delete obj1;
}

void Dynamic_properties() {
    ObjectSystem objectSystem;
    TestObject::registerClassFactory(&objectSystem);

    TestObject *obj1 = ObjectSystem::objectCreate<TestObject>();

    obj1->setProperty("dynamic1", 100); // Set a new dynamic value
    obj1->setProperty("dynamic1", 200); // Override dynamic value

    int value = obj1->property("dynamic1").toInt();
    QCOMPARE(value, 200);

    obj1->setProperty("dynamic1", Variant()); // Delete dynamic property
    QCOMPARE(obj1->property("dynamic1").isValid(), false);

    delete obj1;
}

} REGISTER(ObjectTest)

#include "tst_object.moc"
