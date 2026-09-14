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
#ifndef MESHCOLLIDER_H
#define MESHCOLLIDER_H

#include <collider.h>

#include <mesh.h>

class PhysicMaterial;

class BULLET_EXPORT MeshCollider : public Collider {
    A_OBJECT(MeshCollider, Collider, Components/Physics)

    A_PROPERTIES(
        A_PROPERTY(Mesh *, Shared_Mesh, MeshCollider::mesh, MeshCollider::setMesh)
    )
    A_NOMETHODS()

public:
    MeshCollider();
    ~MeshCollider();

    Mesh *mesh() const;
    void setMesh(Mesh *mesh);

    PhysicMaterial *material() const;
    void setMaterial(PhysicMaterial *material);

private:
    btCollisionShape *shape() override;

    void setEnabled(bool enable) override;

protected:
    Mesh *m_mesh;

    PhysicMaterial *m_material;

};
typedef MeshCollider* MeshColliderPtr;

#endif // MESHCOLLIDER_H
