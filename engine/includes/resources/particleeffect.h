#ifndef PARTICLEEFFECT_H
#define PARTICLEEFFECT_H

#include <resource.h>

#include <material.h>
#include <mesh.h>

#include <deque>
#include <anim/variantanimation.h>

class ParticleData {
public:
    ParticleData();

    /// Current color and alpha of particle
    Vector4 color;
    /// Delta to change color and alpha of particle every second
    Vector4 colrate;
    /// Transformed position of particle
    Vector3 transform;
    /// Current rotation of particle in rads
    Vector3 angle;
    /// Current size of particle
    Vector3 size;
    /// Current position of the particle
    Vector3 position;
    /// Delta to change position of particle every second
    Vector3 velocity;
    /// Delta to change rotation of particle every second in degrees
    Vector3 anglerate;
    /// Delta to change size of particle every second
    Vector3 sizerate;
    /// Current life of the particle in seconds
    float life;
    /// Current animation frame
    float frame;
    /// Distance to camera
    float distance;
};

class ENGINE_EXPORT ParticleModificator {
public:
    enum ValueType {
        CONSTANT    = 0,
        RANGE,
        CURVE
    };

    enum ModificatorType {
        LIFETIME    = 1,
        STARTSIZE,
        STARTCOLOR,
        STARTANGLE,
        STARTPOSITION,

        SCALESIZE   = 50,
        SCALECOLOR,
        SCALEANGLE,
        VELOCITY
    };

public:
    ParticleModificator();
    virtual ~ParticleModificator();

    virtual void spawnParticle(ParticleData &data);
    virtual void updateParticle(ParticleData &data, float dt);

    void loadData(const VariantList &list);

protected:
    ValueType m_type;
    Vector4 m_min;
    Vector4 m_max;

    VariantAnimation m_curve;

};
typedef std::deque<ParticleModificator *> ModifiersDeque;

class ENGINE_EXPORT ParticleEffect : public Resource {
    A_REGISTER(ParticleEffect, Resource, Resources)

    A_PROPERTIES(
        A_PROPERTY(Mesh *, mesh, ParticleEffect::mesh, ParticleEffect::setMesh),
        A_PROPERTY(Material *, material, ParticleEffect::material, ParticleEffect::setMaterial),
        A_PROPERTY(float, distibution, ParticleEffect::distribution, ParticleEffect::setDistribution),
        A_PROPERTY(bool, local, ParticleEffect::local, ParticleEffect::setLocal),
        A_PROPERTY(bool, continous, ParticleEffect::continous, ParticleEffect::setContinous)
    )

public:
    ParticleEffect();
    ~ParticleEffect();

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

    ModifiersDeque &modifiers();
    void setModifiers(const ModifiersDeque &modifiers);

    void loadUserData(const VariantMap &data) override;

protected:
    ModifiersDeque m_modifiers;

    Mesh *m_mesh;

    Material *m_material;

    float m_distibution;

    bool m_gpu;

    bool m_local;

    bool m_continous;

};

#endif // PARTICLEEFFECT_H
