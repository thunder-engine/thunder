/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
#ifndef PHYSICMATERIAL_H
#define PHYSICMATERIAL_H

#include <resource.h>
#include <bullet.h>

class BULLET_EXPORT PhysicMaterial : public Resource {
    A_OBJECT(PhysicMaterial, Resource, Resources)

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
