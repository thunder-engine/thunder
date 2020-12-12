#ifndef ANGELMODULE_H
#define ANGELMODULE_H

#include "engine.h"

class AngelScript : public Object {
    A_REGISTER(AngelScript, Object, Resources)

public:
    void loadUserData (const VariantMap &data) override;
    VariantMap saveUserData() const override;

    ByteArray m_Array;

};

#endif // ANGELMODULE_H
