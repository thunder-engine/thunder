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
