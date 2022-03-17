#include <resources/pose.h>

#define DATA "Data"

Bone::Bone() :
    m_index(0) {

}

bool Bone::operator== (const Bone &bone) const {
    return (m_index == bone.m_index) &&
           (m_position == bone.m_position) &&
           (m_rotation == bone.m_rotation) &&
           (m_scale == bone.m_scale);
}

int Bone::index() const {
    return m_index;
}
void Bone::setIndex(int value) {
    m_index = value;
}

const Vector3 &Bone::position() const {
    return m_position;
}
void Bone::setPosition(const Vector3 value) {
    m_position = value;
}

const Vector3 &Bone::rotation() const {
    return m_rotation;
}
void Bone::setRotation(const Vector3 value) {
    m_rotation = value;
}

const Vector3 &Bone::scale() const {
    return m_scale;
}
void Bone::setScale(const Vector3 value) {
    m_scale = value;
}


class PosePrivate {
public:
    deque<Bone> m_Bones;
};

/*!
    \class Pose
    \brief Representation of pose of an animated character.
    \inmodule Resources
*/

Pose::Pose() :
        p_ptr(new PosePrivate) {

}

Pose::~Pose() {
    delete p_ptr;
}
/*!
    Adds a \a bone to the pose.
*/
void Pose::addBone(Bone *bone) {
    if(bone) {
        p_ptr->m_Bones.push_back(*bone);
    }
}
/*!
    Returns a bone with \a index.
    \note Returns nullptr in case no such bone.
*/
const Bone *Pose::bone(int index) const {
    if(index < (int)p_ptr->m_Bones.size()) {
        return &p_ptr->m_Bones[index];
    }
    return nullptr;
}
/*!
    Returns the count of bones for the current pose which was affected.
*/
int Pose::boneCount() const {
    return p_ptr->m_Bones.size();
}
/*!
    \internal
*/
void Pose::loadUserData(const VariantMap &data) {
    p_ptr->m_Bones.clear();

    auto it = data.find(DATA);
    if(it != data.end()) {
        for(auto &b : (*it).second.value<VariantList>()) {
            VariantList attribs = b.value<VariantList>();
            if(attribs.size() == 4) {
                Bone bone;

                auto i = attribs.begin();
                bone.setPosition(i->toVector3());
                i++;
                bone.setRotation(i->toVector3());
                i++;
                bone.setScale(i->toVector3());
                i++;
                bone.setIndex(int32_t(i->toInt()));

                p_ptr->m_Bones.push_back(bone);
            }
        }
    }
}
/*!
    \internal
*/
VariantMap Pose::saveUserData() const {
    VariantMap result;

    VariantList data;
    for(int32_t i = 0; i < boneCount(); i++) {
        VariantList attribs;

        const Bone *b = bone(i);

        attribs.push_back(b->position());
        attribs.push_back(b->rotation());
        attribs.push_back(b->scale());
        attribs.push_back(int(b->index()));

        data.push_back(attribs);
    }
    result[DATA] = data;

    return result;
}

/*!
    \internal

    \warning Do not call this function manually
*/
void Pose::registerSuper(ObjectSystem *system) {
    REGISTER_META_TYPE(Bone);
    Pose::registerClassFactory(system);
}
