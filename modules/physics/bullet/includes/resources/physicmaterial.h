#ifndef PHYSICMATERIAL_H
#define PHYSICMATERIAL_H

#include "resource.h"

class PhysicMaterial : public Resource {
    A_REGISTER(PhysicMaterial, Resource, Resources)

public:
    PhysicMaterial();

    float friction() const;
    void setFriction(float);

    float restitution() const;
    void setRestitution(float);

    float density() const;
    void setDensity(float);

private:
    void loadUserData(const VariantMap &data) override;

private:
    float m_Friction;
    float m_Restitution;
    float m_Density;

};

#endif // PHYSICMATERIAL_H
