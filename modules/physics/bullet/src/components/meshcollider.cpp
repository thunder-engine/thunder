#include "components/meshcollider.h"

#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>
#include <BulletCollision/CollisionShapes/btTriangleMesh.h>
#include <btBulletDynamicsCommon.h>

#include <components/actor.h>
#include <components/transform.h>

#include <resources/mesh.h>
#include "resources/physicmaterial.h"

MeshCollider::MeshCollider() :
        m_mesh(nullptr),
        m_material(nullptr) {

}

MeshCollider::~MeshCollider() {
    if(m_world) {
        m_world->removeRigidBody(static_cast<btRigidBody *>(m_collisionObject));
    }
}

Mesh *MeshCollider::mesh() const {
    return m_mesh;
}

void MeshCollider::setMesh(Mesh *mesh) {
    m_mesh = mesh;

    if(m_world) {
        m_world->removeRigidBody(static_cast<btRigidBody *>(m_collisionObject));
    }
    destroyShape();

    destroyCollider();

    createCollider();
}

PhysicMaterial *MeshCollider::material() const {
    return m_material;
}

void MeshCollider::setMaterial(PhysicMaterial *material) {
    m_material = material;
    if(m_collisionObject && m_material) {
        btRigidBody *body = static_cast<btRigidBody *>(m_collisionObject);
        body->setFriction(m_material->friction());
        body->setRestitution(m_material->restitution());
    }
}

void MeshCollider::setEnabled(bool enable) {
    if(m_collisionObject && m_world) {
        if(enable) {
            m_world->addRigidBody(static_cast<btRigidBody *>(m_collisionObject));
        } else {
            m_world->removeRigidBody(static_cast<btRigidBody *>(m_collisionObject));
        }
    }
}

void MeshCollider::createCollider() {
    m_collisionObject = new btRigidBody(0.0f, nullptr, shape());

    if(m_collisionObject && m_world) {
        m_world->addRigidBody(static_cast<btRigidBody *>(m_collisionObject));
    }
}

btCollisionShape *MeshCollider::shape() {
    if(m_collisionShape == nullptr && m_mesh != nullptr) {

        Lod *lod = m_mesh->lod(0);
        if(lod != nullptr) {
            btTriangleMesh *triangleMesh = new btTriangleMesh();

            Vector3Vector &v = lod->vertices();
            IndexVector &i = lod->indices();
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

            Transform *t = actor()->transform();

            Vector3 p = t->scale();
            m_collisionShape->setLocalScaling(btVector3(p.x, p.y, p.z));
        }
    }
    return m_collisionShape;
}
