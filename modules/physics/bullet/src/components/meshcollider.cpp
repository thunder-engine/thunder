#include "components/meshcollider.h"

#include <components/actor.h>
#include <components/transform.h>

#include <resources/mesh.h>
#include "resources/physicmaterial.h"

#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>
#include <BulletCollision/CollisionShapes/btTriangleMesh.h>
#include <btBulletDynamicsCommon.h>

/*!
    \class MeshCollider
    \brief The MeshCollider class represents a collider based on a 3D mesh.
    \inmodule Engine

    The MeshCollider class provides a way to create a collider based on a 3D mesh.
    The collider can be attached to a physics world, and its properties, such as the mesh, material, and enabled state, can be manipulated dynamically.
*/

MeshCollider::MeshCollider() :
        m_mesh(nullptr),
        m_material(nullptr) {

}

MeshCollider::~MeshCollider() {
    if(m_world && m_collisionObject) {
        m_world->removeCollisionObject(m_collisionObject);
    }
}
/*!
    Returns a pointer to the mesh used by the collider.
*/
Mesh *MeshCollider::mesh() const {
    return m_mesh;
}
/*!
    Sets the \a mesh for the collider.
    This method recreates the collider's shape and updates its properties.
*/
void MeshCollider::setMesh(Mesh *mesh) {
    m_mesh = mesh;

    if(m_world) {
        m_world->removeCollisionObject(m_collisionObject);
    }
    destroyShape();

    destroyCollider();

    createCollider();
}
/*!
    Returns a pointer to the physical material associated with the collider.
*/
PhysicMaterial *MeshCollider::material() const {
    return m_material;
}
/*!
    Sets the physical material for the collider.
    This method updates the friction and restitution properties of the collider.
*/
void MeshCollider::setMaterial(PhysicMaterial *material) {
    m_material = material;
    if(m_collisionObject && m_material) {
        m_collisionObject->setFriction(m_material->friction());
        m_collisionObject->setRestitution(m_material->restitution());
    }
}
/*!
    Set \a enable or disable the collider in the physics world.
*/
void MeshCollider::setEnabled(bool enable) {
    if(m_collisionObject && m_world) {
        if(enable) {
            m_world->addCollisionObject(m_collisionObject);
        } else {
            m_world->removeCollisionObject(m_collisionObject);
        }
    }
}
/*!
    \internal
    Returns a pointer to the Bullet Physics collision shape associated with the collider.
    The collision shape is created from the mesh.
*/
btCollisionShape *MeshCollider::shape() {
    if(m_collisionShape == nullptr && m_mesh != nullptr) {
        btTriangleMesh *triangleMesh = new btTriangleMesh();

        Vector3Vector &v = m_mesh->vertices();
        IndexVector &i = m_mesh->indices();
        for(size_t index = 0; index < i.size(); index += 3) {
            Vector3 v1 = v[i[index]];
            Vector3 v2 = v[i[index + 1]];
            Vector3 v3 = v[i[index + 2]];

            btVector3 bv1 = btVector3(v1.x, v1.y, v1.z);
            btVector3 bv2 = btVector3(v2.x, v2.y, v2.z);
            btVector3 bv3 = btVector3(v3.x, v3.y, v3.z);

            triangleMesh->addTriangle(bv1, bv2, bv3);
        }

        m_collisionShape = new btBvhTriangleMeshShape(triangleMesh, true);

        Vector3 p = transform()->scale();
        m_collisionShape->setLocalScaling(btVector3(p.x, p.y, p.z));

    }
    return m_collisionShape;
}
