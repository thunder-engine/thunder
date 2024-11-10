#include "particleeffect.h"

#include "material.h"
#include "mesh.h"

namespace {
    const char *gEmitters("Emitters");
}

/*!
    \class ParticleModificator
    \brief The ParticleModificator class represents a base class for particle system modifiers, allowing customization of particle behavior during its lifecycle.
    \inmodule Resources

    The ParticleModificator class serves as a base class for particle system modifiers, providing virtual methods for modifying particle data during spawn and update operations.
    The class also includes methods for loading modification data from a variant list.
    Users can extend this class to create custom particle modifiers by overriding the virtual methods and providing specific modification behavior.
*/

ParticleModificator::ParticleModificator(ParticleEffect *effect) :
        m_attribute(Attribute::Lifetime),
        m_random(Randomness::Off),
        m_effect(effect) {
}
/*!
    Modifies a \a particle attribute when spawn a new particle.
*/
void ParticleModificator::spawnParticle(ParticleTransientData &data, int index) const {
    A_UNUSED(data);
}
/*!
    Virtual method for modifying particle \a data during each frame update.
    Parameter \a dt used to pass delta time from Timer.
*/
void ParticleModificator::updateParticle(std::vector<ParticleTransientData> &data, float dt) const {
    A_UNUSED(data);
    A_UNUSED(dt);
}

/*!
    Sets and \a attribute to modify.
*/
void ParticleModificator::setAttribute(Attribute attribute) {
    m_attribute = attribute;
}
/*!
    Loads serialized modification data from a variant \a list.
*/
void ParticleModificator::loadData(const VariantList &list) {
    auto it = list.begin();
    m_random = static_cast<Randomness>((*it).toInt());
    it++;

    Vector4 min((*it).toVector4());
    it++;

    if(m_random == Randomness::PerComponent) {
        Vector4 max((*it).toVector4());

        m_transientData.resize(m_effect->capacity());

        for(auto &it : m_transientData) {
            it = Vector4(RANGE(min.x, max.x), RANGE(min.y, max.y), RANGE(min.z, max.z), RANGE(min.w, max.w));
        }
    } else {
        m_transientData.push_back(min);
    }
}

class SetAttribute: public ParticleModificator {
public:
    explicit SetAttribute(ParticleEffect *effect) :
        ParticleModificator(effect) {

    }

    void spawnParticle(ParticleTransientData &data, int index) const override {
        if(data.life <= 0.0f) {
            const Vector4 &v = m_transientData[(m_random == Off) ? 0 : index];

            switch(m_attribute) {
                case Lifetime: data.lifetime = v.x; break;
                case Position: data.position = v; break;
                case Rotation: data.rotation.z = v.x * DEG2RAD; break;
                case Color: data.color = v; break;
                case Size: data.size = v; break;
                default: break;
            }
        }
    }
};

class UpdateAttribute: public ParticleModificator {
public:
    explicit UpdateAttribute(ParticleEffect *effect) :
        ParticleModificator(effect) {

    }

    void spawnParticle(ParticleTransientData &data, int index) const override {
        if(m_attribute == Velocity) {
            data.velocity = m_transientData[(m_random == Off) ? 0 : index];
        }
    }

    void updateParticle(std::vector<ParticleTransientData> &data, float dt) const override {
        for(int i = 0; i < data.size(); i++) {
            const Vector4 &v = m_transientData[(m_random == Off) ? 0 : i];

            switch(m_attribute) {
                case Velocity: data[i].position += data[i].velocity * dt; break;
                case ScaleColor: data[i].color += v * dt; break;
                case ScaleSize: data[i].size += v * dt; break;
                case ScaleRotation: data[i].rotation += v * dt; break;
                default: break;
            }
        }
    }
};
/*!
    \class ParticleEffect
    \brief Contains all necessary information about the effect.
    \inmodule Resources

    PartcileEffect alows developer to create or modify a complex particle effects.
*/

ParticleEffect::ParticleEffect() :
        m_mesh(nullptr),
        m_material(nullptr),
        m_distibution(1.0f),
        m_capacity(1),
        m_gpu(false),
        m_local(false),
        m_continous(true) {

}

/*!
    Returns a mesh associated with the particle emitter.
*/
Mesh *ParticleEffect::mesh() const {
    return m_mesh;
}
/*!
    Sets a \a mesh associated with the particle emitter.
*/
void ParticleEffect::setMesh(Mesh *mesh) {
    m_mesh = mesh;
}
/*!
    Returns a material associated with the particle emitter.
*/
Material *ParticleEffect::material() const {
    return m_material;
}
/*!
    Sets a \a material associated with the particle emitter.
*/
void ParticleEffect::setMaterial(Material *material) {
    m_material = material;
}
/*!
    Returns a distribution factor of emitted particles.
*/
float ParticleEffect::distribution() const {
    return m_distibution;
}
/*!
    Sets a \a distribution factor of emitted particles.
*/
void ParticleEffect::setDistribution(float distribution) {
    m_distibution = distribution;
}
/*!
    Returns a maximum number of particles to emit.
*/
int ParticleEffect::capacity() const {
    return m_capacity;
}
/*!
    Sets a maximum \a capacity of particles to emit.
*/
void ParticleEffect::setCapacity(int capacity) {
    m_capacity = capacity;
}
/*!
    Returns true if particles are in local space, false otherwise.
*/
bool ParticleEffect::local() const {
    return m_local;
}
/*!
    Setter for the \a local flag indicating local particle space.
*/
void ParticleEffect::setLocal(bool local) {
    m_local = local;
}
/*!
    Returns true if GPU particle simulation is enabled, false otherwise.

    \note Gpu simulation is not supported yet.
*/
bool ParticleEffect::gpu() const {
    return m_gpu;
}
/*!
    Setter for the \a gpu flag indicating GPU particle simulation.

    \note Gpu simulation is not supported yet.
*/
void ParticleEffect::setGpu(bool gpu) {
    m_gpu = gpu;
}
/*!
    Returns true for continuous emission, false for one time emission.
*/
bool ParticleEffect::continous() const {
    return m_continous;
}
/*!
    Setter for the \a continuous flag indicating continuous particle emission.
*/
void ParticleEffect::setContinous(bool continuous) {
    m_continous = continuous;
}
/*!
    Returns bounding box for the emitter.
*/
AABBox ParticleEffect::bound() const {
    return m_aabb;
}
/*!
    Returns an array of particle modifiers.
*/
const std::vector<ParticleModificator *> &ParticleEffect::modificators() const {
    return m_modificators;
}
/*!
    \internal
*/
void ParticleEffect::loadUserData(const VariantMap &data) {
    PROFILE_FUNCTION();

    for(auto it : m_modificators) {
        delete it;
    }
    m_modificators.clear();

    auto section = data.find(gEmitters);
    if(section != data.end()) {
        VariantList list = (*section).second.value<VariantList>();
        for(auto &e : list) {
            VariantList fields = e.value<VariantList>();
            auto it = fields.begin();

            setMesh(Engine::loadResource<Mesh>((*it).toString()));
            it++;
            setMaterial(Engine::loadResource<Material>((*it).toString()));
            it++;
            setGpu((*it).toBool());
            it++;
            setLocal((*it).toBool());
            it++;
            setContinous((*it).toBool());
            it++;
            setCapacity((*it).toInt());
            it++;
            setDistribution((*it).toFloat());
            it++;

            // Particle spawn attribute modificators
            for(auto &m : (*it).value<VariantList>()) {
                VariantList mods = m.value<VariantList>();
                auto mod = mods.begin();
                ParticleModificator::Attribute type = static_cast<ParticleModificator::Attribute>((*mod).toInt());

                ParticleModificator *modificator = nullptr;
                switch (type) {
                    case ParticleModificator::Lifetime:
                    case ParticleModificator::Size:
                    case ParticleModificator::Color:
                    case ParticleModificator::Rotation:
                    case ParticleModificator::Position: modificator = new SetAttribute(this); break;

                    case ParticleModificator::ScaleSize:
                    case ParticleModificator::ScaleColor:
                    case ParticleModificator::ScaleRotation:
                    case ParticleModificator::Velocity: modificator = new UpdateAttribute(this); break;
                    default: break;
                }

                if(modificator) {
                    mod++;
                    modificator->setAttribute(type);
                    modificator->loadData((*mod).value<VariantList>());
                    m_modificators.push_back(modificator);
                }
            }
        }
    }
}
