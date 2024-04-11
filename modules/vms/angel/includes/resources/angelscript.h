#ifndef ANGELMODULE_H
#define ANGELMODULE_H

#include <resource.h>

class AngelScript : public Resource {
    A_REGISTER(AngelScript, Resource, Resources)

public:
    ~AngelScript();

    void loadUserData (const VariantMap &data) override;
    VariantMap saveUserData() const override;

    ByteArray m_array;

};

#endif // ANGELMODULE_H
