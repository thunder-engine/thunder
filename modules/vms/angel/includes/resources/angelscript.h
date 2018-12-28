#ifndef ANGELMODULE_H
#define ANGELMODULE_H

#include "engine.h"

class NEXT_LIBRARY_EXPORT AngelScript : public Object {
    A_REGISTER(AngelScript, Object, Resources);

public:
    void                        loadUserData                (const VariantMap &data);

    ByteArray                   m_Array;

};

#endif // ANGELMODULE_H
