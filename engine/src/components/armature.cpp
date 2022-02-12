#include "armature.h"

#include "components/transform.h"
#include "components/actor.h"

#include "systems/resourcesystem.h"

#include "resources/pose.h"
#include "resources/texture.h"

#include "commandbuffer.h"

#include <cstring>
#include <cfloat>

#define M4X3_SIZE 48
#define MAX_BONES 170

namespace {
const char *POSE = "Pose";
}

class ArmaturePrivate : public Resource::IObserver {
public:
    ArmaturePrivate() :
            m_pBindPose(nullptr),
            m_pCache(nullptr),
            m_BindDirty(false) {

        m_pCache = ResourceSystem::objectCreate<Texture>();
        m_pCache->setFormat(Texture::RGBA32Float);
        m_pCache->resize(512, 1);

        Matrix4 t;
        Texture::Surface &surface = m_pCache->surface(0);
        ByteArray &array = surface[0];
        int8_t *data = &array[0];
        for(uint32_t i = 0; i < MAX_BONES; i++) {
            memcpy(&data[i * M4X3_SIZE], t.mat, M4X3_SIZE);
        }
    }

    ~ArmaturePrivate() {
        if(m_pBindPose) {
            m_pBindPose->unsubscribe(this);
        }
    }

    void resourceUpdated(const Resource *resource, Resource::ResourceState state) {
        if(resource == m_pBindPose && state == Resource::Ready) {
            m_BindDirty = true;
        }
    }

    void cleanDirty(Actor *actor) {
        if(m_pBindPose) {
            list<Actor *> bones = actor->findChildren<Actor *>();

            uint32_t count = m_pBindPose->boneCount();
            m_Bones.resize(count);
            m_InvertTransform.resize(count);
            m_Transform.resize(count);

            for(uint32_t c = 0; c < count; c++) {
                const Bone *b = m_pBindPose->bone(c);
                for(auto it : bones) {
                    Transform *t = it->transform();
                    if((int)t->clonedFrom() == b->index()) {
                        m_Bones[c] = t;
                        m_InvertTransform[c] = Matrix4(b->position(), b->rotation(), b->scale());
                        break;
                    }
                }
            }
        } else {
            m_Bones.clear();
            m_InvertTransform.clear();
        }
        m_BindDirty = false;
    }
    vector<Matrix4> m_InvertTransform;
    vector<Matrix4> m_Transform;
    vector<Transform *> m_Bones;
    Pose *m_pBindPose;
    Texture *m_pCache;
    bool m_BindDirty;
};

/*!
    \class Armature
    \brief A bone management component.
    \inmodule Engine

    An armature in Thunder Engine can be thought of as similar to the armature of a real skeleton, and just like a real skeleton.
    These bones can be moved around and anything that they are attached to or associated with will move and deform in a similar way.
    An armature uses the Transform component in a child object like a bone structure.
*/

Armature::Armature() :
        p_ptr(new ArmaturePrivate) {

}

Armature::~Armature() {
    delete p_ptr;
    p_ptr = nullptr;
}
/*!
    \internal
*/
void Armature::update() {
    if(p_ptr->m_BindDirty) {
        p_ptr->cleanDirty(actor());
    }

    Texture::Surface &surface = p_ptr->m_pCache->surface(0);
    ByteArray &array = surface[0];
    int8_t *data = &array[0];

    for(uint32_t i = 0; i < p_ptr->m_Bones.size(); i++) {
        if(i < p_ptr->m_InvertTransform.size() && p_ptr->m_Bones[i]) {
            p_ptr->m_Transform[i] = p_ptr->m_Bones[i]->worldTransform() * p_ptr->m_InvertTransform[i];
        }
        Matrix4 t = p_ptr->m_Transform[i];
        t[3]  = p_ptr->m_Transform[i].mat[12];
        t[7]  = p_ptr->m_Transform[i].mat[13];
        t[11] = p_ptr->m_Transform[i].mat[14];

        memcpy(&data[i * M4X3_SIZE], t.mat, M4X3_SIZE);
    }
    p_ptr->m_pCache->setDirty();
}
/*!
    Returns a bind pose of the bone structure.
*/
Pose *Armature::bindPose() const {
    return p_ptr->m_pBindPose;
}
/*!
    Sets a bind (initial) \a pose of the bone structure.
*/
void Armature::setBindPose(Pose *pose) {
    if(p_ptr->m_pBindPose != pose) {
        if(p_ptr->m_pBindPose) {
            p_ptr->m_pBindPose->unsubscribe(p_ptr);
        }
        p_ptr->m_pBindPose = pose;
        p_ptr->m_BindDirty = true;
        p_ptr->m_pBindPose->subscribe(p_ptr);

        update();
    }
}
/*!
    \internal
*/
Texture *Armature::texture() const {
    return p_ptr->m_pCache;
}
/*!
    \internal
*/
AABBox Armature::recalcBounds(const AABBox &aabb) const {
    Vector3 v0, v1;
    aabb.box(v0, v1);

    Vector3 min( FLT_MAX);
    Vector3 max(-FLT_MAX);
    for(uint32_t b = 0; b < p_ptr->m_Bones.size(); b++) {
        Vector3 t0 = p_ptr->m_Transform[b] * v0;
        Vector3 t1 = p_ptr->m_Transform[b] * v1;

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
#ifdef SHARED_DEFINE
#include "viewport/handles.h"

bool Armature::drawHandles(ObjectList &selected) {
    if(isBoneSelected(selected)) {
        for(auto it : p_ptr->m_Bones) {
            Handles::drawBone(it->parentTransform(), it);
        }
    }
    update();

    return false;
}

bool Armature::isBoneSelected(ObjectList &selected) {
    for(auto item : selected) {
        if(actor() == item) {
            return true;
        }
        for(auto bone : p_ptr->m_Bones) {
            if(bone->actor() == item) {
                return true;
            }
        }
    }
    return false;
}
#endif
