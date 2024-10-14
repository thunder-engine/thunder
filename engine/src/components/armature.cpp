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
#define MAX_BONES 170

static std::hash<std::string> hash_str;

namespace {
    const char *gPose = "Pose";
}

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
        m_cache(nullptr),
        m_bindDirty(false) {

    m_cache = ResourceSystem::objectCreate<Texture>("ArmatureCache");
    m_cache->setFormat(Texture::RGBA32Float);
    m_cache->resize(512, 1);

    Matrix4 t;
    Texture::Surface &surface = m_cache->surface(0);
    ByteArray &array = surface.front();
    uint8_t *data = array.data();
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
    ByteArray &array = surface.front();
    uint8_t *data = array.data();

    Matrix4 localInv(transform()->worldTransform().inverse());

    for(uint32_t i = 0; i < m_bones.size(); i++) {
        if(i < m_invertTransform.size() && m_bones[i]) {
            Matrix4 t(localInv * m_bones[i]->worldTransform() * m_invertTransform[i]);

            // Compress data
            t[3]  = t[12];
            t[7]  = t[13];
            t[11] = t[14];

            memcpy(&data[i * M4X3_SIZE], t.mat, M4X3_SIZE);
        }
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
        if(m_bindPose) {
            m_bindPose->subscribe(&Armature::bindPoseUpdated, this);
        }

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
void Armature::loadUserData(const VariantMap &data) {
    Component::loadUserData(data);

    auto it = data.find(gPose);
    if(it != data.end()) {
        setBindPose(Engine::loadResource<Pose>((*it).second.toString()));
    }
}
/*!
    \internal
*/
VariantMap Armature::saveUserData() const {
    VariantMap result = Component::saveUserData();

    std::string ref = Engine::reference(bindPose());
    if(!ref.empty()) {
        result[gPose] = ref;
    }

    return result;
}

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

void Armature::cleanDirty(Actor *actor) {
    if(m_bindPose) {
        std::list<Actor *> bones = actor->findChildren<Actor *>();

        uint32_t count = m_bindPose->boneCount();
        m_bones.reserve(count);
        m_invertTransform.resize(count);

        for(uint32_t c = 0; c < count; c++) {
            const Bone *b = m_bindPose->bone(c);
            for(auto it : bones) {
                int hash = hash_str(it->name());
                if(hash == b->index()) {
                    m_bones.push_back(it->transform());
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
    switch(state) {
    case Resource::Ready: {
        static_cast<Armature *>(ptr)->m_bindDirty = true;
    } break;
    case Resource::ToBeDeleted: {
        Armature *armature = static_cast<Armature *>(ptr);
        armature->m_bindPose = nullptr;
    } break;
    default: break;
    }
}
