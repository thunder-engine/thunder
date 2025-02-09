#ifndef POSTPROCESSVOLUME_H
#define POSTPROCESSVOLUME_H

#include "component.h"

class PostProcessSettings;

class ENGINE_EXPORT PostProcessVolume : public Component {
    A_REGISTER(PostProcessVolume, Component, Components/Volumes)

    A_PROPERTIES(
        A_PROPERTY(int, priority, PostProcessVolume::priority, PostProcessVolume::setPriority),
        A_PROPERTY(bool, unbound, PostProcessVolume::unbound, PostProcessVolume::setUnbound)
    )
    A_NOMETHODS()

public:
    PostProcessVolume();
    ~PostProcessVolume();

    int priority() const;
    void setPriority(int priority);

    bool unbound() const;
    void setUnbound(bool unbound);

    float blendWeight() const;
    void setBlendWeight(float weight);

    AABBox bound() const;

    const PostProcessSettings *settings() const;

private:
    void setProperty(const char *name, const Variant &value) override;

    void setSystem(ObjectSystem *system) override;

    void drawGizmos() override;

private:
    PostProcessSettings *m_settings;

    int32_t m_priority;

    float m_blendWeight;

    bool m_unbound;

};

#endif // POSTPROCESSVOLUME_H
