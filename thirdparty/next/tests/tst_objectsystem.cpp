#include "tst_common.h"

#include "objectsystem.h"

#include "json.h"
#include "bson.h"

TestObject *resource = nullptr;

static ObjectSystem objectSystem;

class SecondObject : public TestObject {
    A_REGISTER(SecondObject, TestObject, Test)

    A_NOMETHODS()
    A_NOPROPERTIES()

};

class TestObjectEx : public TestObject {
    A_OVERRIDE(TestObjectEx, TestObject, Test)

    A_NOMETHODS()
    A_NOPROPERTIES()
};

class ObjectSystemTest : public QObject {
    Q_OBJECT
private slots:

void initTestCase() {
    resource   = ObjectSystem::objectCreate<TestObject>();
    QCOMPARE((resource != nullptr), true);
    resource->setName("TestResource");
}

void cleanupTestCase() {
    delete resource;
}

void RegisterUnregister_Object() {
    QCOMPARE((int)objectSystem.factories().size(), 1);
    SecondObject::registerClassFactory(&objectSystem);
    QCOMPARE((int)objectSystem.factories().size(), 2);
    SecondObject::unregisterClassFactory(&objectSystem);
    QCOMPARE((int)objectSystem.factories().size(), 1);
}

void Object_Instansing() {
    TestObject obj1;
    Object *result1 = ObjectSystem::objectCreate<TestObject>();
    Object *object  = dynamic_cast<Object*>(&obj1);

    QCOMPARE((result1 != nullptr), true);
    QCOMPARE((object != nullptr), true);
    QCOMPARE(compare(*object, *result1), true);

    Object *result2 = ObjectSystem::objectCreate<TestObject>();

    QCOMPARE((result1->uuid() != result2->uuid()), true);

    delete result1;
    delete result2;
}

void Override_Object() {
    TestObjectEx::registerClassFactory(&objectSystem);

    Object *object  = ObjectSystem::objectCreate<TestObject>();

    QCOMPARE((object != nullptr), true);
    const MetaObject *meta = object->metaObject();

    QCOMPARE((dynamic_cast<TestObjectEx *>(object) != nullptr), true);
    QCOMPARE(meta->methodCount(), 3);
    QCOMPARE(meta->propertyCount(), 3);

    int index   = meta->indexOfProperty("slot");
    QCOMPARE((index > -1), true);
    delete object;

    TestObjectEx::unregisterClassFactory(&objectSystem);

    object = ObjectSystem::objectCreate<TestObject>();
    QCOMPARE((dynamic_cast<TestObject *>(object) != nullptr), true);
    QCOMPARE((dynamic_cast<TestObjectEx *>(object) == nullptr), true);
    delete object;
}

void Serialize_Desirialize_Object() {
    TestObject *obj1 = ObjectSystem::objectCreate<TestObject>();
    TestObject *obj2 = ObjectSystem::objectCreate<TestObject>();
    TestObject *obj3 = ObjectSystem::objectCreate<TestObject>();

    obj1->setName("MainObject");

    obj2->setName("TestComponent2");
    obj3->setName("TestComponent3");
    obj2->setParent(obj1);
    obj3->setParent(obj2);

    QCOMPARE(Object::connect(obj1, _SIGNAL(signal(int)), obj2, _SLOT(setSlot(int))), true);
    QCOMPARE(Object::connect(obj1, _SIGNAL(signal(int)), obj3, _SIGNAL(signal(int))), true);
    QCOMPARE(Object::connect(obj2, _SIGNAL(signal(int)), obj3, _SLOT(setSlot(int))), true);

    ByteArray bytes = Bson::save(ObjectSystem::toVariant(obj1));
    Object *result  = ObjectSystem::toObject(Bson::load(bytes));
    Object *object  = dynamic_cast<Object*>(obj1);

    QCOMPARE((result != nullptr), true);
    QCOMPARE((object != nullptr), true);
    QCOMPARE(compare(*object, *result), true);

    QCOMPARE((object->getReceivers().size() == result->getReceivers().size()), true);

    QCOMPARE((object->uuid() == result->uuid()), true);

    delete result;

    delete obj3;
    delete obj2;
    delete obj1;
}

} REGISTER(ObjectSystemTest)

#include "tst_objectsystem.moc"
