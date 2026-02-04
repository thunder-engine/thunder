#include <resources/pose.h>

namespace  {
    const char *gData("Data");
}

/*!
    \class Bone
    \brief The Bone class represents a bone in a skeletal animation system.
    \inmodule Resources
*/

Bone::Bone() {

}

Bone::~Bone() {

}
/*!
    Overloaded equality operator for comparing two \a bone objects.
    Returns true if the bones are equal, false otherwise.
*/
bool Bone::operator== (const Bone &bone) const {
    return (m_name == bone.m_name) &&
           (m_position == bone.m_position) &&
           (m_rotation == bone.m_rotation) &&
           (m_scale == bone.m_scale);
}
/*!
    Returns the name of the bone.
*/
TString Bone::name() const {
    return m_name;
}
/*!
    Sets the \a name of the bone.
*/
void Bone::setName(const TString &name) {
    m_name = name;
}
/*!
    Gets the position of the bone.
*/
const Vector3 &Bone::position() const {
    return m_position;
}
/*!
    Sets the \a position of the bone.
*/
void Bone::setPosition(const Vector3 position) {
    m_position = position;
}
/*!
    Gets the rotation of the bone.
*/
const Vector3 &Bone::rotation() const {
    return m_rotation;
}
/*!
    Sets the \a rotation of the bone.
*/
void Bone::setRotation(const Vector3 rotation) {
    m_rotation = rotation;
}
/*!
    Gets the scale of the bone.
*/
const Vector3 &Bone::scale() const {
    return m_scale;
}
/*!
    Sets the \a scale of the bone.
*/
void Bone::setScale(const Vector3 scale) {
    m_scale = scale;
}


/*!
    \class Pose
    \brief Representation of pose of an animated character.
    \inmodule Resources
*/

Pose::Pose() {

}

Pose::~Pose() {

}
/*!
    Removes all bones from pose.
*/
void Pose::clear() {
    m_bones.clear();
}
/*!
    Adds a \a bone to the pose.
*/
void Pose::addBone(const Bone &bone) {
    m_bones.push_back(bone);
}
/*!
    Returns a bone with \a index.
    \note Returns nullptr in case no such bone.
*/
const Bone *Pose::bone(int index) const {
    if(index < (int)m_bones.size()) {
        return &m_bones[index];
    }
    return nullptr;
}
/*!
    Returns the count of bones for the current pose which was affected.
*/
int Pose::boneCount() const {
    return m_bones.size();
}
/*!
    \internal
*/
void Pose::loadUserData(const VariantMap &data) {
    clear();

    auto it = data.find(gData);
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
                bone.setName(i->toString());

                m_bones.push_back(bone);
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
        attribs.push_back(b->name());

        data.push_back(attribs);
    }
    result[gData] = data;

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
