#ifndef TST_COMMON_H
#define TST_COMMON_H

#include <QTest>
#include <QDebug>

#include "object.h"

#include "objectsystem.h"

class TestObject : public Object {
    A_REGISTER(TestObject, Object, Test)

    A_METHODS(
        A_SLOT(TestObject::setSlot),
        A_SIGNAL(TestObject::signal),
        A_METHOD(bool, TestObject::test)
    )

    A_PROPERTIES(
        A_PROPERTY(bool, slot, TestObject::getSlot, TestObject::setSlot),
        A_PROPERTY(Vector2, vec, TestObject::getVector, TestObject::setVector),
        A_PROPERTY(TestObject *, resource, TestObject::getResource, TestObject::setResource)
    )

public:
    explicit TestObject     () :
            Object() {
        m_bSlot     = false;
        m_Vector2   = Vector2(1.0f, 0.0f);
        m_pResource = nullptr;

    }

    bool            getSlot         () const {
        return m_bSlot;
    }

    bool            test            () {
        return false;
    }

    void            setSlot         (const int value) {
        m_bSlot     = value;
    }

    Vector2         getVector       () const {
        return m_Vector2;
    }

    void            setVector       (const Vector2 &value) {
        m_Vector2   = value;
    }

    TestObject     *getResource     () const {
        return m_pResource;
    }

    void            setResource     (TestObject *resource) {
        m_pResource = resource;
    }

    void            signal          (const int);

    int             m_bSlot;
    Vector2         m_Vector2;
    TestObject     *m_pResource;
};

inline bool compare(const Object::Link &left, const Object::Link &right) {
    bool result = true;
    result &= left.signal   == right.signal;
    result &= left.method   == right.method;
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
            Variant lv  = lp.read(&left);
            Variant rv  = rp.read(&right);
            if(lp.name() != rp.name() || lv != rv) {
                return false;
            }
        }
    }

    {
        if(left.getReceivers().size() == right.getReceivers().size()) {
            for(const auto &li : left.getReceivers()) {
                result  = false;
                for(const auto &ri : right.getReceivers()) {
                    if(compare(li, ri)) {
                        result  = true;
                        break;
                    }
                }
                if(!result) {
                    return result;
                }
            }
        } else {
            return false;
        }
    }

    {
        if(left.getChildren().size() == right.getChildren().size()) {
            auto il = left.getChildren().begin();
            auto ir = right.getChildren().begin();
            while(il != left.getChildren().end() && ir != right.getChildren().end()) {
                Object *l  = *il;
                Object *r  = *ir;
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

class TestRunnder {
public:
    static QObjectList &tests() {
        return TestRunnder::m_List;
    }

    static int runAllTests(int argc, char **argv) {
        int status = 0;
        for(auto it : m_List) {
            status |= QTest::qExec(it, argc, argv);
        }
        qDebug() << "********* All tests completed *********";
        return status;
    }
private:
    static QObjectList m_List;
};

class ListAdder {
public:
    ListAdder(QObjectList &list, QObject *item) {
        list.push_back(item);
    }
};

#define REGISTER(Name)              \
static test_ ## Name ## Instance;   \
                                    \
static ListAdder adder ## Name (TestRunnder::tests(), &test_ ## Name ## Instance);


#endif // TST_COMMON_H
