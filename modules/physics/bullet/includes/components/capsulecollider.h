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
#ifndef CAPSULECOLLIDER_H
#define CAPSULECOLLIDER_H

#include "spherecollider.h"

class BULLET_EXPORT CapsuleCollider : public SphereCollider {
    A_OBJECT(CapsuleCollider, SphereCollider, Components/Physics)

    A_PROPERTIES(
        A_PROPERTY(float, height, CapsuleCollider::height, CapsuleCollider::setHeight)
    )
    A_NOMETHODS()

public:
    CapsuleCollider();

    float height() const;
    void setHeight(float height);

private:
    void drawGizmosSelected() override;

    btCollisionShape *shape() override;

protected:
    float m_height;

};
typedef CapsuleCollider* CapsuleColliderPtr;

#endif // CAPSULECOLLIDER_H
