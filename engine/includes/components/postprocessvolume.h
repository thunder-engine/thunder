#ifndef POSTPROCESSVOLUME_H
#define POSTPROCESSVOLUME_H

#include "component.h"

class PostProcessSettings;

class ENGINE_EXPORT PostProcessVolume : public Component {
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

    const PostProcessSettings &settings() const;

    static const MetaObject *metaClass();

public:
    static void registerClassFactory(ObjectSystem *system);
    static void unregisterClassFactory(ObjectSystem *system);

private:
    static Object *construct();

    const MetaObject *metaObject() const override;

    Variant readProperty(const MetaProperty &property) const;

    void writeProperty(const MetaProperty &property, const Variant value);

    void drawGizmos() override;

private:
    std::vector<MetaProperty::Table> m_propertyTable;

    PostProcessSettings *m_settings;

    MetaObject *m_metaObject;

    int32_t m_priority;

    float m_blendWeight;

    bool m_unbound;

};

#endif // POSTPROCESSVOLUME_H
