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

class ArmaturePrivate {
public:
    ArmaturePrivate() :
            m_BindDirty(false),
            m_pBindPose(nullptr),
            m_pCache(nullptr) {

        m_pCache = ResourceSystem::objectCreate<Texture>();
        m_pCache->setFormat(Texture::RGBA32Float);
        m_pCache->resize(512, 1);

        Matrix4 t;
        Texture::Surface &surface = m_pCache->surface(0);
        uint8_t *data = surface[0];
        for(uint32_t i = 0; i < MAX_BONES; i++) {
            memcpy(&data[i * M4X3_SIZE], t.mat, M4X3_SIZE);
        }
    }

    void cleanDirty(Actor *actor) {
        if(m_pBindPose) {
            list<Actor *> bones = actor->findChildren<Actor *>();
            bones.push_front(actor);

            m_Bones.resize(bones.size());

            uint32_t i = 0;
            for(auto it : bones) {
                m_Bones[i] = it->transform();
                i++;
            }

            uint32_t count = m_pBindPose->size();
            m_InvertTransform.resize(m_Bones.size());
            m_Transform.resize(m_Bones.size());

            Matrix4 p;
            for(i = 0; i < m_Bones.size(); i++) {
                if(i < count) {
                    const Pose::Bone *bone = m_pBindPose->bone(i);
                    Matrix4 m = (i == 0) ? p : p * Matrix4(bone->position, bone->rotation, bone->scale);
                    m_InvertTransform[i] = m.inverse();
                    p = m;
                }
            }
        } else {
            m_Bones.clear();
            m_InvertTransform.clear();
        }
        m_BindDirty = false;
    }
    bool m_BindDirty;
    Pose *m_pBindPose;
    Texture *m_pCache;
    vector<Transform *> m_Bones;
    vector<Matrix4> m_InvertTransform;
    vector<Matrix4> m_Transform;
};

Armature::Armature() :
        p_ptr(new ArmaturePrivate) {

}

Armature::~Armature() {
    delete p_ptr;
}

void Armature::draw(ICommandBuffer &buffer, uint32_t layer) {
    A_UNUSED(buffer);

    if(layer == ICommandBuffer::DEFAULT) {
        if(p_ptr->m_BindDirty) {
            p_ptr->cleanDirty(actor());
        }

        Texture::Surface &surface = p_ptr->m_pCache->surface(0);
        uint8_t *data = surface[0];

        for(uint32_t i = 0; i < p_ptr->m_Bones.size(); i++) {
            if(i < p_ptr->m_InvertTransform.size()) {
                Matrix4 world = p_ptr->m_Bones[i]->worldTransform();
                p_ptr->m_Transform[i] = world * p_ptr->m_InvertTransform[i];
            }
            Matrix4 t = p_ptr->m_Transform[i];
            t[3]  = p_ptr->m_Transform[i].mat[12];
            t[7]  = p_ptr->m_Transform[i].mat[13];
            t[11] = p_ptr->m_Transform[i].mat[14];

            memcpy(&data[i * M4X3_SIZE], t.mat, M4X3_SIZE);
        }
        p_ptr->m_pCache->setDirty();
    }
}

Pose *Armature::bindPose() const {
    return p_ptr->m_pBindPose;
}

void Armature::setBindPose(Pose *pose) {
    if(p_ptr->m_pBindPose != pose) {
        p_ptr->m_pBindPose = pose;
        p_ptr->m_BindDirty = true;
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
    Vector3 v[2];
    aabb.box(v[0], v[1]);

    Vector3 min( FLT_MAX);
    Vector3 max(-FLT_MAX);
    // This implementation is not accurate I guess it should take into account weight of the bones
    for(uint32_t i = 0; i < p_ptr->m_Bones.size(); i++) {
        Vector3 v0 = p_ptr->m_Transform[i] * v[0];
        Vector3 v1 = p_ptr->m_Transform[i] * v[1];

        min.x = MIN(min.x, v0.x);
        min.y = MIN(min.y, v0.y);
        min.z = MIN(min.z, v0.z);

        min.x = MIN(min.x, v1.x);
        min.y = MIN(min.y, v1.y);
        min.z = MIN(min.z, v1.z);

        max.x = MAX(max.x, v0.x);
        max.y = MAX(max.y, v0.y);
        max.z = MAX(max.z, v0.z);

        max.x = MAX(max.x, v1.x);
        max.y = MAX(max.y, v1.y);
        max.z = MAX(max.z, v1.z);
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
#ifdef NEXT_SHARED
#include "handles.h"

bool Armature::drawHandles(ObjectList &selected) {
    if(isBoneSelected(selected)) {
        for(auto it : p_ptr->m_Bones) {
            Handles::drawBone(it->parentTransform()->worldTransform(), it->worldTransform());
        }
    }
    return false;
}

bool Armature::isBoneSelected(ObjectList &selected) {
    for(auto item : selected) {
        for(auto bone : p_ptr->m_Bones) {
            if(bone->actor() == item) {
                return true;
            }
        }
    }
    return false;
}
#endif
