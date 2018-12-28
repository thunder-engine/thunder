#ifndef ANGELBEHAVIOUR_H
#define ANGELBEHAVIOUR_H

#include "components/nativebehaviour.h"

class asIScriptObject;
class asIScriptFunction;

class AngelBehaviour : public NativeBehaviour {
    A_REGISTER(AngelBehaviour, NativeBehaviour, Components)

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
    void                        setScriptStart          (asIScriptFunction *function);

    asIScriptFunction          *scriptUpdate            () const;
    void                        setScriptUpdate         (asIScriptFunction *function);

protected:
    string                      m_Script;

    asIScriptObject            *m_pObject;

    asIScriptFunction          *m_pStart;
    asIScriptFunction          *m_pUpdate;

};

#endif // ANGELBEHAVIOUR_H
