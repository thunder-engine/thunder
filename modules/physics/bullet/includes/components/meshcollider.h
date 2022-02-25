#ifndef MESHCOLLIDER_H
#define MESHCOLLIDER_H

#include "collider.h"

class Mesh;
class PhysicMaterial;

class MeshCollider : public Collider {
    A_REGISTER(MeshCollider, Collider, Components/Physics)

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

    void createCollider() override;

    void setEnabled(bool enable) override;

protected:
    Mesh *m_mesh;

    PhysicMaterial *m_material;

};

#endif // MESHCOLLIDER_H
