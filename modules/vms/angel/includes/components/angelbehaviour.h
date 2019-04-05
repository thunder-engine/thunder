#ifndef ANGELBEHAVIOUR_H
#define ANGELBEHAVIOUR_H

#include "components/nativebehaviour.h"

class asIScriptObject;
class asIScriptFunction;

class AngelBehaviour : public NativeBehaviour {
    A_PROPERTIES(
        A_PROPERTY(string, Script, AngelBehaviour::script, AngelBehaviour::setScript)
    )

    A_NOMETHODS()
public:
    AngelBehaviour              ();

    virtual ~AngelBehaviour     ();

    string                      script                  () const;
    void                        setScript               (const string &value);

    asIScriptObject            *scriptObject            () const;
    void                        setScriptObject         (asIScriptObject *object);

    asIScriptFunction          *scriptStart             () const;

    asIScriptFunction          *scriptUpdate            () const;

    const MetaObject           *metaObject              () const;

public:
    static void                 registerClassFactory    (ObjectSystem *system) {
        REGISTER_META_TYPE(AngelBehaviour);
        system->factoryAdd<AngelBehaviour>("Components", AngelBehaviour::metaClass());
    }
    static void                 unregisterClassFactory  (ObjectSystem *system) {
        UNREGISTER_META_TYPE(AngelBehaviour);
        system->factoryRemove<AngelBehaviour>("Components");
    }

private:
    static Object *construct() { return new AngelBehaviour(); }

    void setScriptStart(asIScriptFunction *function);
    void setScriptUpdate(asIScriptFunction *function);

    VariantList saveData() const;
    void loadData(const VariantList &data);

    VariantMap saveUserData() const;
    void loadUserData(const VariantMap &data);

public:
    static const MetaObject *metaClass() {
        OBJECT_CHECK(AngelBehaviour)
        static const MetaObject staticMetaData (
            "AngelBehaviour",
            NativeBehaviour::metaClass(),
            &AngelBehaviour::construct,
            expose_method<AngelBehaviour>::exec(),
            expose_props_method<AngelBehaviour>::exec()
        );
        return &staticMetaData;
    }

protected:
    string                      m_Script;

    asIScriptObject            *m_pObject;

    asIScriptFunction          *m_pStart;
    asIScriptFunction          *m_pUpdate;

    vector<MetaProperty::Table> m_Table;

    MetaObject                 *m_pMetaObject;
};

#endif // ANGELBEHAVIOUR_H
