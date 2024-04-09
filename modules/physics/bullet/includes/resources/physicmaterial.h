#ifndef PHYSICMATERIAL_H
#define PHYSICMATERIAL_H

#include <resource.h>
#include <bullet.h>

class BULLET_EXPORT PhysicMaterial : public Resource {
    A_REGISTER(PhysicMaterial, Resource, Resources)

    A_PROPERTIES(
        A_PROPERTY(float, friction, PhysicMaterial::friction, PhysicMaterial::setFriction),
        A_PROPERTY(float, restitution, PhysicMaterial::restitution, PhysicMaterial::setRestitution),
        A_PROPERTY(float, density, PhysicMaterial::density, PhysicMaterial::setDensity)
    )
    A_NOMETHODS()

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
    VariantMap saveUserData() const override;

private:
    float m_Friction;
    float m_Restitution;
    float m_Density;

};
typedef PhysicMaterial* PhysicMaterialPtr;

#endif // PHYSICMATERIAL_H
