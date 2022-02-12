#ifndef MESHCOLLIDER_H
#define MESHCOLLIDER_H

#include "collider.h"

class Mesh;
class btTriangleMesh;

class MeshCollider : public Collider {
    A_REGISTER(MeshCollider, Collider, Components/Physics)

    A_PROPERTIES(
        A_PROPERTY(Mesh *, Shared_Mesh, MeshCollider::mesh, MeshCollider::setMesh)
    )
    A_NOMETHODS()

public:
    MeshCollider();

    Mesh *mesh() const;
    void setMesh(Mesh *mesh);

private:
    btCollisionShape *shape() override;

    void createCollider() override;

protected:
    Mesh *m_mesh;

    btTriangleMesh *m_triangleMesh;
};

#endif // MESHCOLLIDER_H
