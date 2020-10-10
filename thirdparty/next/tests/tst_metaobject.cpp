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

class SecondObject : public TestObject {
    A_REGISTER(SecondObject, TestObject, Test)

    A_NOMETHODS() //  If not set this the method count will be doubled
    A_NOPROPERTIES()

    A_ENUMS(
        A_ENUM(TestEnum,
               A_VALUE(TestValue0),
               A_VALUE(TestValue1))
    )

    enum TestEnum {
        TestValue0,
        TestValue1 = 2
    };
};

class MetaObjectTest : public QObject {
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

    int type = MetaType::type<TestObject *>();
    bool result = MetaType::registerConverter(type, MetaType::VARIANTLIST, &MetaObjectTest::toList);

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

    TestObject::unregisterClassFactory(&objectSystem);
}

void Meta_property() {
    ObjectSystem objectSystem;

    SecondObject::registerClassFactory(&objectSystem);
    SecondObject obj;

    const MetaObject *meta = obj.metaObject();
    QVERIFY(meta != nullptr);

    QCOMPARE(meta->name(), "SecondObject");

    QCOMPARE(meta->propertyCount(), 4);
    QCOMPARE(meta->property(0).isValid(), true);
    QCOMPARE(meta->property(1).isValid(), true);

    obj.setSlot(false);
    Variant v = meta->property(0).read(&obj);
    QCOMPARE(v.toBool(), false);
    obj.setSlot(true);
    QCOMPARE(meta->property(0).read(&obj).toBool(), obj.getSlot());

    {
        bool value = false;
        meta->property(0).write(&obj, value);
        QCOMPARE(obj.getSlot(), value);
    }
    {
        Vector2 value(1.0, 2.0);
        meta->property(2).write(&obj, value);
        QCOMPARE(obj.getVector().x, value.x);
        QCOMPARE(obj.getVector().y, value.y);
    }

    int index = meta->indexOfProperty("IntProperty");
    QCOMPARE(index > -1, true);
    MetaProperty property = meta->property(index);
    QCOMPARE(property.isValid(), true);
    QCOMPARE(property.type().name(), "int");

    SecondObject::unregisterClassFactory(&objectSystem);
}

void Meta_methods() {
    SecondObject obj;
    const MetaObject *meta = obj.metaObject();
    QVERIFY(meta != nullptr);

    QCOMPARE(meta->methodCount(), 7);
    int index = meta->indexOfSlot("setSlot(int)");
    QCOMPARE(index > -1, true);

    MetaMethod method = meta->method(index);
    QCOMPARE(method.isValid(), true);
    Variant value;
    QCOMPARE(obj.getSlot(), false);

    Variant arg(true);
    QCOMPARE(method.invoke(&obj, value, 1, &arg), true);
    QCOMPARE(obj.getSlot(), true);

    QCOMPARE(meta->indexOfSignal("setSlot"), -1);

    index = meta->indexOfSignal("signal(int)");
    QCOMPARE(index > -1, true);

    index = meta->indexOfMethod("testInt()");
    QCOMPARE(index > -1, true);
    method = meta->method(index);
    QCOMPARE(method.isValid(), true);

    QCOMPARE(method.returnType().name(), "int");
}

void Meta_enums() {
    SecondObject obj;

    const MetaObject *meta = obj.metaObject();
    QVERIFY(meta != nullptr);

    QCOMPARE(meta->enumeratorCount(), 1);
    int index = meta->indexOfEnumerator("TestEnum");
    QCOMPARE(index > -1, true);

    MetaEnum enumerator = meta->enumerator(index);
    QCOMPARE(enumerator.isValid(), true);

    QCOMPARE(enumerator.keyCount(), 2);
    QCOMPARE(enumerator.key(0), "TestValue0");
    QCOMPARE(enumerator.value(1), 2);
}

} REGISTER(MetaObjectTest)

#include "tst_metaobject.moc"
