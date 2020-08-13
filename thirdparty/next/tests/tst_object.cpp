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

static bool toList(void *to, const void *from, const uint32_t fromType) {
    if(fromType == MetaType::type<TestObject *>()) {
        const Object *o = *(const Object **)from;

        VariantList *r = static_cast<VariantList *>(to);
        *r = ObjectSystem::toVariant(o).value<VariantList>();

        return true;
    }
    return false;
}

void Meta_type() {
    ObjectSystem objectSystem;
    TestObject::registerClassFactory(&objectSystem);

    int type    = MetaType::type<TestObject *>();
    bool result = MetaType::registerConverter(type, MetaType::VARIANTLIST, &ObjectTest::toList);

    QCOMPARE((type > 0), true);
    QCOMPARE(result, true);

    TestObject obj;
    Variant variant = Variant::fromValue(&obj);

    QCOMPARE(variant.isValid(), true);
    QCOMPARE((int)variant.userType(), type);

    VariantList list = variant.toList();
    QCOMPARE((int)list.size(), 1);

    type = MetaType::type<TestObject>();
    size_t size = sizeof(TestObject);
    void *where = malloc(size);
    MetaType::construct(type, where);
    TestObject *r = reinterpret_cast<TestObject *>(where);
    QCOMPARE(r->getVector(), Vector2(1.0f, 0.0f));
    MetaType::destruct(type, where);
}

void Meta_property() {
    TestObject obj;
    const MetaObject *meta  = obj.metaObject();
    QVERIFY(meta != nullptr);

    QCOMPARE(meta->name(), "TestObject");

    QCOMPARE(meta->propertyCount(), 3);
    QCOMPARE(meta->property(0).isValid(), true);
    QCOMPARE(meta->property(1).isValid(), true);

    obj.setSlot(false);
    Variant v = meta->property(0).read(&obj);
    QCOMPARE(v.toBool(), false);
    obj.setSlot(true);
    QCOMPARE(meta->property(0).read(&obj).toBool(), obj.getSlot());

    {
        bool value  = false;
        meta->property(0).write(&obj, value);
        QCOMPARE(obj.getSlot(), value);
    }
    {
        Vector2 value(1.0, 2.0);
        meta->property(1).write(&obj, value);
        QCOMPARE(obj.getVector().x, value.x);
        QCOMPARE(obj.getVector().y, value.y);
    }
}

void Meta_methods() {
    TestObject obj;
    const MetaObject *meta = obj.metaObject();
    QVERIFY(meta != nullptr);

    QCOMPARE(meta->methodCount(), 5);

    int index = meta->indexOfSlot("setSlot(int)");
    QCOMPARE(index > -1, true);

    MetaMethod method   = meta->method(index);
    QCOMPARE(method.isValid(), true);
    Variant value;
    QCOMPARE(obj.getSlot(), false);

    Variant arg(true);
    QCOMPARE(method.invoke(&obj, value, 1, &arg), true);
    QCOMPARE(obj.getSlot(), true);

    QCOMPARE(meta->indexOfSignal("setSlot"), -1);

    index   = meta->indexOfSignal("signal(int)");
    QCOMPARE(index > -1, true);
}

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
    obj1->setProperty("dynamic1", 100);
    obj2->setProperty("dynamic2", true);

    Object::connect(obj1, _SIGNAL(signal(int)), obj2, _SLOT(setSlot(int)));
    Object::connect(obj2, _SIGNAL(signal(int)), obj1, _SLOT(setSlot(int)));

    Object *clone = obj1->clone();
    QCOMPARE((clone != nullptr), true);
    QCOMPARE(compare(*clone, *obj1), true);
    QCOMPARE((clone->uuid() != 0), true);
    delete clone;

    delete obj2;
    delete obj1;
}

} REGISTER(ObjectTest)

#include "tst_object.moc"
