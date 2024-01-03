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

    VariantAnimation m_Curve;

};
typedef std::deque<ParticleModificator *> ModifiersDeque;

class ENGINE_EXPORT ParticleEmitter {
    A_PROPERTIES(
        A_PROPERTY(Mesh *, mesh, ParticleEmitter::mesh, ParticleEmitter::setMesh),
        A_PROPERTY(Material *, material, ParticleEmitter::material, ParticleEmitter::setMaterial),
        A_PROPERTY(float, distibution, ParticleEmitter::distribution, ParticleEmitter::setDistribution),
        A_PROPERTY(bool, local, ParticleEmitter::local, ParticleEmitter::setLocal),
        A_PROPERTY(bool, continous, ParticleEmitter::continous, ParticleEmitter::setContinous)
    )
    A_NOMETHODS()

public:
    ParticleEmitter();

    bool operator== (const ParticleEmitter &emitter) const;

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

private:
    ModifiersDeque m_modifiers;

    Mesh *m_mesh;

    Material *m_material;

    float m_distibution;

    bool m_gpu;

    bool m_local;

    bool m_continous;

};
typedef deque<ParticleEmitter> EmitterDeque;

class ENGINE_EXPORT ParticleEffect : public Resource {
    A_REGISTER(ParticleEffect, Resource, Resources)

    A_NOPROPERTIES()
    A_METHODS(
        A_METHOD(void, ParticleEffect::clear),
        A_METHOD(int, ParticleEffect::emittersCount),
        A_METHOD(ParticleEmitter *, ParticleEffect::emitter),
        A_METHOD(void, ParticleEffect::addEmitter)
    )

public:
    ParticleEffect();
    ~ParticleEffect();

    void clear();

    int emittersCount() const;

    ParticleEmitter *emitter(int index);
    void addEmitter(ParticleEmitter *emitter);

    void loadUserData(const VariantMap &data) override;

    static void registerSuper(ObjectSystem *system);

protected:
    EmitterDeque m_Emitters;

};

#endif // PARTICLEEFFECT_H
