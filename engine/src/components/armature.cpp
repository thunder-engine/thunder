#include "armature.h"

#include "components/transform.h"
#include "components/actor.h"

#include "resources/mesh.h"
#include "resources/pose.h"
#include "resources/texture.h"

#include "systems/resourcesystem.h"

#include "commandbuffer.h"
#include "gizmos.h"

#include <cstring>
#include <cfloat>

#define M4X3_SIZE 48

/*!
    \class Armature
    \brief A bone management component.
    \inmodule Components

    An armature in Thunder Engine can be thought of as similar to the armature of a real skeleton, and just like a real skeleton.
    These bones can be moved around and anything that they are attached to or associated with will move and deform in a similar way.
    An armature uses the Transform component in a child object like a bone structure.
*/

Armature::Armature() :
        m_bindPose(nullptr),
        m_bindDirty(false) {

}

Armature::~Armature() {
    if(m_bindPose) {
        m_bindPose->unsubscribe(this);
    }
}

void Armature::addInstance(MaterialInstance *instance) {
    if(instance) {
        instance->setSkinSize(m_bones.size() * M4X3_SIZE);
        m_instances.push_back(instance);
    }
}

void Armature::removeInstance(MaterialInstance *instance) {
    m_instances.remove(instance);
}
/*!
    \internal
*/
void Armature::update() {
    if(m_bindDirty) {
        cleanDirty();
    }

    Transform *t = transform();
    if(t) {
        Matrix4 localInv(t->worldTransform().inverse());

        for(uint32_t i = 0; i < m_bones.size(); i++) {
            if(i < m_invertTransform.size() && m_bones[i]) {
                Matrix4 mat(localInv * m_bones[i]->worldTransform() * m_invertTransform[i]);

                // Compress data
                mat[3]  = mat[12];
                mat[7]  = mat[13];
                mat[11] = mat[14];

                for(auto it : m_instances) {
                    ByteArray &uniformBuffer = it->rawUniformBuffer();
                    uint8_t *data = uniformBuffer.data();
                    uint32_t offset = it->material()->uniformSize();

                    memcpy(&data[offset + i * M4X3_SIZE], mat.mat, M4X3_SIZE);
                }
            }
        }
    }
}
/*!
    Returns a bind pose of the bone structure.
*/
Pose *Armature::bindPose() const {
    return m_bindPose;
}
/*!
    \fn void Armature::setBindPose(Pose *pose)

    Sets a bind (initial) \a pose of the bone structure.
*/
void Armature::setBindPose(Pose *pose) {
    if(m_bindPose != pose) {
        if(m_bindPose) {
            m_bindPose->unsubscribe(this);
        }

        m_bindPose = pose;
        m_bindDirty = true;
        if(m_bindPose) {
            m_bindPose->subscribe(&Armature::bindPoseUpdated, this);
        }

        update();
    }
}
/*!
    \internal
*/
void Armature::drawGizmosSelected() {
    static Mesh *bone = nullptr;
    if(bone == nullptr) {
        bone = Engine::loadResource<Mesh>(".embedded/bone.fbx/Bone");
    }

    if(bone) {
        Vector4 color(0.0f, 1.0f, 0.0f, 0.1f);
        for(auto it : m_bones) {
            Transform *p = it->parentTransform();
            Vector3 parent(p->worldPosition());
            Gizmos::drawMesh(*bone, color, Matrix4(parent, p->worldQuaternion(), Vector3((it->worldPosition() - parent).length())));
        }
    }
}
/*!
    \internal
*/
void Armature::cleanDirty() {
    if(m_bindPose) {
        std::list<Actor *> bones = actor()->findChildren<Actor *>();

        uint32_t count = m_bindPose->boneCount();
        m_bones.reserve(count);
        m_invertTransform.reserve(count);

        for(uint32_t c = 0; c < count; c++) {
            const Bone *b = m_bindPose->bone(c);
            for(auto it : bones) {
                int hash = Mathf::hashString(it->name());
                if(hash == b->index()) {
                    m_bones.push_back(it->transform());
                    m_invertTransform.push_back(Matrix4(b->position(), b->rotation(), b->scale()));
                    break;
                }
            }
        }

        for(auto it : m_instances) {
            it->setSkinSize(m_bones.size() * M4X3_SIZE);
        }
    } else {
        m_bones.clear();
        m_invertTransform.clear();
    }

    m_bindDirty = false;
}
/*!
    \internal
*/
void Armature::bindPoseUpdated(int state, void *ptr) {
    switch(state) {
    case Resource::Ready: {
        static_cast<Armature *>(ptr)->m_bindDirty = true;
    } break;
    case Resource::ToBeDeleted: {
        static_cast<Armature *>(ptr)->m_bindPose = nullptr;
    } break;
    default: break;
    }
}
