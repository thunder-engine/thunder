#include <resources/pose.h>

#define DATA "Data"

class PosePrivate {
public:
    deque<Pose::Bone> m_Bones;
};

Pose::Pose() :
        p_ptr(new PosePrivate) {

}

Pose::~Pose() {
    delete p_ptr;
}

void Pose::addBone(const Pose::Bone &bone) {
    p_ptr->m_Bones.push_back(bone);
}

const Pose::Bone *Pose::bone(uint32_t index) const {
    if(index < p_ptr->m_Bones.size()) {
        return &p_ptr->m_Bones[index];
    }
    return nullptr;
}

uint32_t Pose::size() const {
    return p_ptr->m_Bones.size();
}

void Pose::loadUserData(const VariantMap &data) {
    p_ptr->m_Bones.clear();

    auto it = data.find(DATA);
    if(it != data.end()) {
        for(auto b : (*it).second.value<VariantList>()) {
            VariantList attribs = b.value<VariantList>();
            if(attribs.size() == 4) {
                Bone bone;

                auto i = attribs.begin();
                bone.position = i->toVector3();
                i++;
                bone.rotation = i->toVector3();
                i++;
                bone.scale = i->toVector3();
                i++;
                bone.index = uint32_t(i->toInt());

                p_ptr->m_Bones.push_back(bone);
            }
        }
    }

    setState(Ready);
}
