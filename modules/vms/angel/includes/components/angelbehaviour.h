#ifndef ANGELBEHAVIOUR_H
#define ANGELBEHAVIOUR_H

#include "components/nativebehaviour.h"

class asIScriptObject;
class asIScriptFunction;

class AngelBehaviour : public NativeBehaviour {
    A_PROPERTIES(
        A_PROPERTYEX(TString, script, AngelBehaviour::script, AngelBehaviour::setScript, "ReadOnly")
    )

    A_NOMETHODS()
public:
    AngelBehaviour();
    ~AngelBehaviour();

    TString script() const;
    void setScript(const TString value);

    asIScriptObject *scriptObject() const;
    void setScriptObject(asIScriptObject *object);

    asIScriptFunction *scriptStart() const;
    asIScriptFunction *scriptUpdate() const;

    void createObject();

public:
    static void registerClassFactory(ObjectSystem *system);
    static void unregisterClassFactory(ObjectSystem *system);

private:
    friend class AngelSystem;

    static void *construct() { return new AngelBehaviour(); }

    void setScriptStart(asIScriptFunction *function);
    void setScriptUpdate(asIScriptFunction *function);

    const MetaObject *metaObject() const override;

    VariantList saveData() const override;
    void loadData(const VariantList &data) override;

    void setType(const TString &type) override;
    void setSystem(ObjectSystem *system) override;

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
            NativeBehaviour::metaClass(),
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

    asIScriptObject *m_object;

    asIScriptFunction *m_start;
    asIScriptFunction *m_update;

};

#endif // ANGELBEHAVIOUR_H
