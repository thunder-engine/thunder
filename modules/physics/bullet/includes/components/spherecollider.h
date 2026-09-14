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
#ifndef SPHERECOLLIDER_H
#define SPHERECOLLIDER_H

#include "volumecollider.h"

class BULLET_EXPORT SphereCollider : public VolumeCollider {
    A_OBJECT(SphereCollider, VolumeCollider, Components/Physics)

    A_PROPERTIES(
        A_PROPERTY(float, radius, SphereCollider::radius, SphereCollider::setRadius)
    )
    A_NOMETHODS()

public:
    SphereCollider();

    float radius() const;
    void setRadius(float radius);

private:
    void drawGizmosSelected() override;

    btCollisionShape *shape() override;

protected:
    float m_radius;

};
typedef SphereCollider* SphereColliderPtr;

#endif // SPHERECOLLIDER_H
