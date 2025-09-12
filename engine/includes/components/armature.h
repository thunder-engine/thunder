#ifndef ARMATURE_H
#define ARMATURE_H

#include <nativebehaviour.h>

#include <pose.h>

class MaterialInstance;

class ENGINE_EXPORT Armature : public NativeBehaviour {
    A_OBJECT(Armature, NativeBehaviour, Components/Animation);

    A_PROPERTIES(
        A_PROPERTYEX(Pose *, bindPose, Armature::bindPose, Armature::setBindPose, "editor=Asset")
    )
    A_NOMETHODS()

public:
    Armature();
    ~Armature() override;

    Pose *bindPose() const;
    void setBindPose(Pose *pose);

    void addInstance(MaterialInstance *instance);
    void removeInstance(MaterialInstance *instance);

    void update() override;

private:
    void drawGizmosSelected() override;

    void cleanDirty();

    static void bindPoseUpdated(int state, void *ptr);

private:
    std::vector<Matrix4> m_invertTransform;
    std::vector<Transform *> m_bones;
    std::list<MaterialInstance *> m_instances;

    Pose *m_bindPose;

    bool m_bindDirty;

};

#endif // ARMATURE_H
