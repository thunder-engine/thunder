#include "components/meshcollider.h"

#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>
#include <BulletCollision/CollisionShapes/btTriangleMesh.h>
#include <btBulletDynamicsCommon.h>

#include <components/actor.h>
#include <components/transform.h>

#include "resources/physicmaterial.h"

MeshCollider::MeshCollider() :
        m_mesh(nullptr),
        m_triangleMesh(new btTriangleMesh()) {

}

Mesh *MeshCollider::mesh() const {
    return m_mesh;
}

void MeshCollider::setMesh(Mesh *mesh) {
    m_mesh = mesh;
}

btCollisionShape *MeshCollider::shape() {
    if(m_pCollisionShape == nullptr) {
        m_pCollisionShape = new btBvhTriangleMeshShape(m_triangleMesh, true);

        Transform *t = actor()->transform();

        Vector3 p = t->scale();
        m_pCollisionShape->setLocalScaling(btVector3(p.x, p.y, p.z));
    }
    return m_pCollisionShape;
}

void MeshCollider::createCollider() {
    m_pCollisionObject = new btRigidBody(0.0f, nullptr, m_pCollisionShape);

    if(m_pCollisionObject && m_pWorld) {
        m_pWorld->addRigidBody(static_cast<btRigidBody *>(m_pCollisionObject));
    }
}
