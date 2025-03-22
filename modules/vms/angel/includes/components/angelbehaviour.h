#ifndef ANGELBEHAVIOUR_H
#define ANGELBEHAVIOUR_H

#include "components/nativebehaviour.h"

class asIScriptObject;
class asIScriptFunction;

class AngelBehaviour : public NativeBehaviour {
    A_PROPERTIES(
        A_PROPERTYEX(string, script, AngelBehaviour::script, AngelBehaviour::setScript, "ReadOnly")
    )

    A_NOMETHODS()
public:
    AngelBehaviour();
    ~AngelBehaviour();

    std::string script() const;
    void setScript(const std::string value);

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

    void updateMeta();

    void setScriptStart(asIScriptFunction *function);
    void setScriptUpdate(asIScriptFunction *function);

    const MetaObject *metaObject() const override;

    VariantList saveData() const override;
    void loadData(const VariantList &data) override;

    VariantMap saveUserData() const override;
    void loadUserData(const VariantMap &data) override;

    void setType(const std::string &type) override;
    void setSystem(ObjectSystem *system) override;

    void scriptSlot();

    void onReferenceDestroyed() override;

    Variant readProperty(const MetaProperty &property) const;
    void writeProperty(const MetaProperty &property, const Variant value);

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
    std::string m_script;

    std::list<std::pair<AngelBehaviour *, void *>> m_obsevers;
    std::vector<MetaProperty::Table> m_propertyTable;
    std::vector<MetaMethod::Table> m_methodTable;

    struct PropertyFields {
        Object *object;
        void *address;
        bool isObject;
        bool isScript;
    };

    std::unordered_map<const char *, PropertyFields> m_propertyAdresses;

    asIScriptObject *m_object;

    asIScriptFunction *m_start;
    asIScriptFunction *m_update;

    MetaObject *m_metaObject;
};

#endif // ANGELBEHAVIOUR_H
