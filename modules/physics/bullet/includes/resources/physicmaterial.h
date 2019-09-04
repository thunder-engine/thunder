#ifndef PHYSICMATERIAL_H
#define PHYSICMATERIAL_H

#include "resource.h"

class PhysicMaterial : public Resource {
    A_REGISTER(PhysicMaterial, Resource, Resources)

public:
    PhysicMaterial ();

    float friction () const;
    void setFriction (float);

    float bounciness () const;
    void setBounciness (float);

private:
    void loadUserData (const VariantMap &data) override;

private:
    float m_Friction;
    float m_Bounciness;

};

#endif // PHYSICMATERIAL_H
