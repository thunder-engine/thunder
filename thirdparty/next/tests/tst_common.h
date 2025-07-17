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

    Copyright: 2008-2025 Evgeniy Prikazchikov
*/

#ifndef TST_COMMON_H
#define TST_COMMON_H

#include "gtest/gtest.h"

#include "object.h"

#include "objectsystem.h"

class TestObject : public Object {
    A_OBJECT(TestObject, Object, Test)

    A_METHODS(
        A_SLOT(TestObject::setSlot),
        A_SLOT(TestObject::onDestroyed),
        A_SIGNAL(TestObject::signal),
        A_METHOD(bool, TestObject::test),
        A_METHOD(int, TestObject::testInt),
        A_METHOD(String, TestObject::testString)
    )

    A_PROPERTIES(
        A_PROPERTY(bool, slot, TestObject::getSlot, TestObject::setSlot),
        A_PROPERTY(int, IntProperty, TestObject::intProperty, TestObject::setIntProperty),
        A_PROPERTY(Vector2, vec, TestObject::getVector, TestObject::setVector),
        A_PROPERTY(TestObject *, resource, TestObject::getResource, TestObject::setResource)
    )

public:
    TestObject() :
            Object() {
        m_bSlot = false;
        m_Integer = 0;
        m_Vector2 = Vector2(1.0f, 0.0f);
        m_pResource = nullptr;

        setName("TestObject");
    }

    bool getSlot() const {
        return m_bSlot;
    }

    bool test() {
        return false;
    }

    int testInt() {
        return 1;
    }

    int intProperty() const {
        return m_Integer;
    }

    void setIntProperty(int value) {
        m_Integer = value;
    }

    TString testString(const TString data) const {
        return data;
    }

    void setSlot(const int value) {
        m_bSlot = value;
    }

    Vector2 getVector() const {
        return m_Vector2;
    }

    void setVector(const Vector2 value) {
        m_Vector2 = value;
    }

    TestObject *getResource() const {
        return m_pResource;
    }

    void setResource(TestObject *resource) {
        m_pResource = resource;
    }

    void onDestroyed() {
        m_bSlot = true;
    }

    void signal(const int);

    int         m_bSlot;
    int         m_Integer;
    Vector2     m_Vector2;
    TestObject *m_pResource;
};

inline bool compare(const Object::Link &left, const Object::Link &right) {
    bool result = true;
    result &= left.signal == right.signal;
    result &= left.method == right.method;
    return result;
}


inline bool compare(const Object &left, const Object &right) {
    bool result = true;

    result &= (left.typeName() == right.typeName());

    if(!result) {
        return result;
    }

    {
        for(int i = 0; i < left.metaObject()->propertyCount(); i++) {
            MetaProperty lp = left.metaObject()->property(i);
            MetaProperty rp = right.metaObject()->property(i);
            Variant lv = lp.read(&left);
            Variant rv = rp.read(&right);
            if(lp.name() != rp.name() || lv != rv) {
                return false;
            }
        }
    }

    {
        if(left.getChildren().size() == right.getChildren().size()) {
            auto il = left.getChildren().begin();
            auto ir = right.getChildren().begin();
            while(il != left.getChildren().end() && ir != right.getChildren().end()) {
                Object *l = *il;
                Object *r = *ir;
                if(!compare(*l, *r)) {
                    return false;
                }
                il++;
                ir++;
            }
        } else {
            return false;
        }
    }

    return true;
}

#endif // TST_COMMON_H
