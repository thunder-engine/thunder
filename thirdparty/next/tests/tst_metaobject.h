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

class MetaObjectTest : public ::testing::Test {
public:
    static bool toList(void *to, const void *from, const uint32_t fromType) {
        if(fromType == MetaType::type<TestObject *>()) {
            const Object *o = *(const Object **)from;

            VariantList *r = static_cast<VariantList *>(to);
            *r = ObjectSystem::toVariant(o).value<VariantList>();

            return true;
        }
        return false;
    }

};

TEST_F(MetaObjectTest, Meta_type) {
    ObjectSystem objectSystem;
    TestObject::registerClassFactory(&objectSystem);

    int type = MetaType::type<TestObject *>();
    bool result = MetaType::registerConverter(type, MetaType::VARIANTLIST, &MetaObjectTest::toList);

    ASSERT_TRUE(type > 0);
    ASSERT_TRUE(result);

    TestObject obj;
    Variant variant = Variant::fromValue(&obj);

    ASSERT_TRUE(variant.isValid());
    ASSERT_TRUE((int)variant.userType() == type);

    VariantList list = variant.toList();
    ASSERT_TRUE((int)list.size() == 1);

    type = MetaType::type<TestObject>();
    size_t size = sizeof(TestObject);
    void *where = malloc(size);
    MetaType::construct(type, where);
    TestObject *r = reinterpret_cast<TestObject *>(where);
    ASSERT_TRUE(r->getVector() == Vector2(1.0f, 0.0f));
    MetaType::destruct(type, where);

    TestObject::unregisterClassFactory(&objectSystem);
}

TEST_F(MetaObjectTest, Meta_property) {
    ObjectSystem objectSystem;

    SecondObject::registerClassFactory(&objectSystem);
    SecondObject obj;

    const MetaObject *meta = obj.metaObject();
    ASSERT_TRUE(meta != nullptr);

    ASSERT_TRUE(std::string(meta->name()) == std::string("SecondObject"));

    ASSERT_TRUE(meta->propertyCount() == 4);
    ASSERT_TRUE(meta->property(0).isValid());
    ASSERT_TRUE(meta->property(1).isValid());

    obj.setSlot(false);
    Variant v = meta->property(0).read(&obj);
    ASSERT_FALSE(v.toBool());
    obj.setSlot(true);
    ASSERT_TRUE(meta->property(0).read(&obj).toBool() == obj.getSlot());

    {
        bool value = false;
        meta->property(0).write(&obj, value);
        ASSERT_TRUE(obj.getSlot() == value);
    }
    {
        Vector2 value(1.0, 2.0);
        meta->property(2).write(&obj, value);
        ASSERT_TRUE(obj.getVector().x == value.x);
        ASSERT_TRUE(obj.getVector().y == value.y);
    }

    int index = meta->indexOfProperty("IntProperty");
    ASSERT_TRUE(index > -1);
    MetaProperty property = meta->property(index);
    ASSERT_TRUE(property.isValid());
    ASSERT_TRUE(std::string(property.type().name()) == std::string("int"));

    SecondObject::unregisterClassFactory(&objectSystem);
}

TEST_F(MetaObjectTest, Meta_methods) {
    SecondObject obj;
    const MetaObject *meta = obj.metaObject();
    ASSERT_TRUE(meta != nullptr);

    ASSERT_TRUE(meta->methodCount() == 7);
    int index = meta->indexOfSlot("setSlot(int)");
    ASSERT_TRUE(index > -1);

    MetaMethod method = meta->method(index);
    ASSERT_TRUE(method.isValid());
    Variant value;
    ASSERT_FALSE(obj.getSlot());

    Variant arg(true);
    ASSERT_TRUE(method.invoke(&obj, value, 1, &arg));
    ASSERT_TRUE(obj.getSlot());

    ASSERT_TRUE(meta->indexOfSignal("setSlot") == -1);

    index = meta->indexOfSignal("signal(int)");
    ASSERT_TRUE(index > -1);

    index = meta->indexOfMethod("testInt()");
    ASSERT_TRUE(index > -1);
    method = meta->method(index);
    ASSERT_TRUE(method.isValid());

    ASSERT_TRUE(std::string(method.returnType().name()) == std::string("int"));
}

TEST_F(MetaObjectTest, Meta_enums) {
    SecondObject obj;

    const MetaObject *meta = obj.metaObject();
    ASSERT_TRUE(meta != nullptr);

    ASSERT_TRUE(meta->enumeratorCount() == 1);
    int index = meta->indexOfEnumerator("TestEnum");
    ASSERT_TRUE(index > -1);

    MetaEnum enumerator = meta->enumerator(index);
    ASSERT_TRUE(enumerator.isValid());

    ASSERT_TRUE(enumerator.keyCount() == 2);
    ASSERT_TRUE(std::string(enumerator.key(0)) == std::string("TestValue0"));
    ASSERT_TRUE(enumerator.value(1) == 2);
}
