/*
    This file is part of Thunder Engine.

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
#ifndef ANGELBEHAVIOUR_H
#define ANGELBEHAVIOUR_H

#include "components/component.h"

class asITypeInfo;
class asIScriptObject;
class asIScriptFunction;

class AngelBehaviour : public Component {
    A_NOPROPERTIES()
    A_NOMETHODS()

public:
    AngelBehaviour();
    ~AngelBehaviour();

    bool isStarted() const;
    void start();
    void update() const;

    asIScriptObject *scriptObject() const;
    void setScriptObject(asIScriptObject *object);

    void destroyObject();
    void createObject();

    void hibernateObject();
    void awakeObject();

public:
    static void registerClassFactory(ObjectSystem *system);
    static void unregisterClassFactory(ObjectSystem *system);

private:
    friend class AngelSystem;

    static void *construct() { return new AngelBehaviour(); }

    void setScriptStart(asIScriptFunction *function);
    void setScriptUpdate(asIScriptFunction *function);

    const MetaObject *metaObject() const override;

    Object *cloneStructure(Object::ObjectPairs &pairs) override;

    VariantList saveData() const override;
    void loadData(const VariantList &data) override;

    void setType(const TString &type) override;

    void scriptSlot();

    void onReferenceDestroyed() override;

    Variant readProperty(const MetaProperty &property) const;
    void writeProperty(const MetaProperty &property, const Variant &value);

    void methodCallEvent(MethodCallEvent *event) override;

    void subscribe(AngelBehaviour *observer, void *ptr);

    void notifyObservers();

public:
    static const MetaObject *metaClass() {
        OBJECT_CHECK(AngelBehaviour)
        static const MetaObject staticMetaData(
            "AngelBehaviour",
            Component::metaClass(),
            &AngelBehaviour::construct,
            reinterpret_cast<const MetaMethod::Table *>(expose_method<AngelBehaviour>::exec()),
            reinterpret_cast<const MetaProperty::Table *>(expose_props_method<AngelBehaviour>::exec()),
            reinterpret_cast<const MetaEnum::Table *>(expose_enum<AngelBehaviour>::exec())
        );
        return &staticMetaData;
    }

protected:
    TString m_script;

    struct PropertyFields {
        Object *object = nullptr;
        void *address = nullptr;
        bool isObject = false;
        bool isScript = false;
        bool isArray = false;
    };

    std::unordered_map<const char *, PropertyFields> m_propertyFields;

    std::list<std::pair<AngelBehaviour *, void *>> m_obsevers;

    VariantMap m_data;

    asIScriptObject *m_object;

    asIScriptFunction *m_start;
    asIScriptFunction *m_update;

    bool m_started;

};

#endif // ANGELBEHAVIOUR_H
