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
#ifndef BOXCOLLIDER_H
#define BOXCOLLIDER_H

#include "volumecollider.h"

class BULLET_EXPORT BoxCollider : public VolumeCollider {
    A_OBJECT(BoxCollider, VolumeCollider, Components/Physics)

    A_PROPERTIES(
        A_PROPERTY(Vector3, size, BoxCollider::size, BoxCollider::setSize)
    )
    A_NOMETHODS()

public:
    BoxCollider();

    const Vector3 &size() const;
    void setSize(const Vector3 size);

private:
    void drawGizmosSelected() override;

    btCollisionShape *shape() override;

protected:
    Vector3 m_size;

};
typedef BoxCollider *BoxColliderPtr;

#endif // BOXCOLLIDER_H
