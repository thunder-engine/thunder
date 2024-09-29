#ifndef PARTICLEEFFECT_H
#define PARTICLEEFFECT_H

#include <resource.h>

#include <material.h>
#include <mesh.h>

#include <deque>
#include <anim/variantanimation.h>

class ParticleModificator;

struct ParticleTransientData {
    Vector4 color;

    Vector4 uv;

    Vector3 velocity;

    float mass;

    Vector3 position;

    float life;

    Vector3 size;

    float lifetime;

    Vector3 rotation;

    float distance;

};

class ENGINE_EXPORT ParticleEffect : public Resource {
    A_REGISTER(ParticleEffect, Resource, Resources)

    A_PROPERTIES(
        A_PROPERTY(int, capacity, ParticleEffect::capacity, ParticleEffect::setCapacity),
        A_PROPERTY(Mesh *, mesh, ParticleEffect::mesh, ParticleEffect::setMesh),
        A_PROPERTY(Material *, material, ParticleEffect::material, ParticleEffect::setMaterial),
        A_PROPERTY(float, distibution, ParticleEffect::distribution, ParticleEffect::setDistribution),
        A_PROPERTY(bool, local, ParticleEffect::local, ParticleEffect::setLocal),
        A_PROPERTY(bool, continous, ParticleEffect::continous, ParticleEffect::setContinous)
    )

public:
    ParticleEffect();

    int capacity() const;
    void setCapacity(int capacity);

    Mesh *mesh() const;
    void setMesh(Mesh *mesh);

    Material *material() const;
    void setMaterial(Material *material);

    float distribution() const;
    void setDistribution(float distibution);

    bool local() const;
    void setLocal(bool local);

    bool gpu() const;
    void setGpu(bool gpu);

    bool continous() const;
    void setContinous(bool continuous);

    AABBox bound() const;

    const std::vector<ParticleModificator *> &modificators() const;

    void loadUserData(const VariantMap &data) override;

protected:
    std::vector<ParticleModificator *> m_modificators;

    AABBox m_aabb;

    Mesh *m_mesh;

    Material *m_material;

    float m_distibution;

    int m_capacity;

    bool m_gpu;

    bool m_local;

    bool m_continous;

};

class ENGINE_EXPORT ParticleModificator {
public:
    enum Randomness {
        Off = 0,
        PerComponent
    };

    enum Attribute {
        Lifetime    = 1,
        Position,
        Velocity,
        Rotation,
        Size,
        Color,

        ScaleSize,
        ScaleColor,
        ScaleRotation
    };

public:
    explicit ParticleModificator(ParticleEffect *effect);

    virtual void spawnParticle(ParticleTransientData &data, int index) const;
    virtual void updateParticle(std::vector<ParticleTransientData> &data, float dt) const;

    void setAttribute(Attribute attribute);

    void loadData(const VariantList &list);

protected:
    std::vector<Vector4> m_transientData;

    Attribute m_attribute;

    Randomness m_random;

    ParticleEffect *m_effect;

};

#endif // PARTICLEEFFECT_H
