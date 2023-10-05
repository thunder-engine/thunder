#include "armature.h"

#include "components/transform.h"
#include "components/actor.h"

#include "resources/pose.h"
#include "resources/texture.h"

#include "systems/resourcesystem.h"

#include "commandbuffer.h"
#include "gizmos.h"

#include <cstring>
#include <cfloat>

#define M4X3_SIZE 48
#define MAX_BONES 170

namespace {
const char *POSE = "Pose";
}

/*!
    \class Armature
    \brief A bone management component.
    \inmodule Engine

    An armature in Thunder Engine can be thought of as similar to the armature of a real skeleton, and just like a real skeleton.
    These bones can be moved around and anything that they are attached to or associated with will move and deform in a similar way.
    An armature uses the Transform component in a child object like a bone structure.
*/

Armature::Armature() :
        m_bindPose(nullptr),
        m_cache(nullptr),
        m_bindDirty(false) {

    m_cache = ResourceSystem::objectCreate<Texture>();
    m_cache->setFormat(Texture::RGBA32Float);
    m_cache->resize(512, 1);

    Matrix4 t;
    Texture::Surface &surface = m_cache->surface(0);
    ByteArray &array = surface[0];
    int8_t *data = &array[0];
    for(uint32_t i = 0; i < MAX_BONES; i++) {
        memcpy(&data[i * M4X3_SIZE], t.mat, M4X3_SIZE);
    }
}

Armature::~Armature() {
    if(m_bindPose) {
        m_bindPose->unsubscribe(this);
    }
}
/*!
    \internal
*/
void Armature::update() {
    if(m_bindDirty) {
        cleanDirty(actor());
    }

    Texture::Surface &surface = m_cache->surface(0);
    ByteArray &array = surface[0];
    int8_t *data = array.data();

    for(uint32_t i = 0; i < m_bones.size(); i++) {
        if(i < m_invertTransform.size() && m_bones[i]) {
            m_transform[i] = m_bones[i]->worldTransform() * m_invertTransform[i];
        }
        // Compress data
        Matrix4 t = m_transform[i];
        t[3]  = t[12];
        t[7]  = t[13];
        t[11] = t[14];

        memcpy(&data[i * M4X3_SIZE], t.mat, M4X3_SIZE);
    }
    m_cache->setDirty();
}
/*!
    Returns a bind pose of the bone structure.
*/
Pose *Armature::bindPose() const {
    return m_bindPose;
}
/*!
    Sets a bind (initial) \a pose of the bone structure.
*/
void Armature::setBindPose(Pose *pose) {
    if(m_bindPose != pose) {
        if(m_bindPose) {
            m_bindPose->unsubscribe(this);
        }
        m_bindPose = pose;
        m_bindDirty = true;
        m_bindPose->subscribe(&Armature::bindPoseUpdated, this);

        update();
    }
}
/*!
    \internal
*/
Texture *Armature::texture() const {
    return m_cache;
}
/*!
    \internal
*/
AABBox Armature::recalcBounds(const AABBox &aabb) const {
    Vector3 v0, v1;
    aabb.box(v0, v1);

    Vector3 min( FLT_MAX);
    Vector3 max(-FLT_MAX);
    for(uint32_t b = 0; b < m_bones.size(); b++) {
        Vector3 t0 = m_transform[b] * v0;
        Vector3 t1 = m_transform[b] * v1;

        min.x = MIN(min.x, t0.x);
        min.y = MIN(min.y, t0.y);
        min.z = MIN(min.z, t0.z);

        min.x = MIN(min.x, t1.x);
        min.y = MIN(min.y, t1.y);
        min.z = MIN(min.z, t1.z);

        max.x = MAX(max.x, t0.x);
        max.y = MAX(max.y, t0.y);
        max.z = MAX(max.z, t0.z);

        max.x = MAX(max.x, t1.x);
        max.y = MAX(max.y, t1.y);
        max.z = MAX(max.z, t1.z);
    }
    AABBox result;
    result.setBox(min, max);

    return result;
}
/*!
    \internal
*/
void Armature::loadUserData(const VariantMap &data) {
    Component::loadUserData(data);
    {
        auto it = data.find(POSE);
        if(it != data.end()) {
            setBindPose(Engine::loadResource<Pose>((*it).second.toString()));
        }
    }
}
/*!
    \internal
*/
VariantMap Armature::saveUserData() const {
    VariantMap result = Component::saveUserData();
    {
        string ref = Engine::reference(bindPose());
        if(!ref.empty()) {
            result[POSE] = ref;
        }
    }
    return result;
}

void Armature::drawGizmosSelected() {
    for(auto it : m_bones) {
        //Handles::drawBone(it->parentTransform(), it);
    }
}

void Armature::cleanDirty(Actor *actor) {
    if(m_bindPose) {
        list<Actor *> bones = actor->findChildren<Actor *>();

        uint32_t count = m_bindPose->boneCount();
        m_bones.resize(count);
        m_invertTransform.resize(count);
        m_transform.resize(count);

        for(uint32_t c = 0; c < count; c++) {
            const Bone *b = m_bindPose->bone(c);
            for(auto it : bones) {
                Transform *t = it->transform();
                if((int)t->clonedFrom() == b->index()) {
                    m_bones[c] = t;
                    m_invertTransform[c] = Matrix4(b->position(), b->rotation(), b->scale());
                    break;
                }
            }
        }
    } else {
        m_bones.clear();
        m_invertTransform.clear();
    }

    m_bindDirty = false;
}

void Armature::bindPoseUpdated(int state, void *ptr) {
    if(state == ResourceState::Ready) {
        static_cast<Armature *>(ptr)->m_bindDirty = true;
    }
}
