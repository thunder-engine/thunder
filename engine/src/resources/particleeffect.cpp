#include "particleeffect.h"

#include "material.h"
#include "mesh.h"

namespace {
    const char *gEmitters = "Emitters";
}

/*!
    \class ParticleModificator
    \brief The ParticleModificator class represents a base class for particle system modifiers, allowing customization of particle behavior during its lifecycle.
    \inmodule Resources

    The ParticleModificator class serves as a base class for particle system modifiers, providing virtual methods for modifying particle data during spawn and update operations.
    The class also includes methods for loading modification data from a variant list.
    Users can extend this class to create custom particle modifiers by overriding the virtual methods and providing specific modification behavior.
*/

ParticleModificator::ParticleModificator() :
        m_type(CONSTANT),
        m_min(1.0f),
        m_max(1.0f) {
}

ParticleModificator::~ParticleModificator() {

}
/*!
    Virtual method for modifying particle \a data during particle spawn.
*/
void ParticleModificator::spawnParticle(ParticleData &data) {
    A_UNUSED(data);
}
/*!
    Virtual method for modifying particle \a data during each frame update.
    Parameter \a dt used to pass delta time from Timer.
*/
void ParticleModificator::updateParticle(ParticleData &data, float dt) {
    A_UNUSED(data);
    A_UNUSED(dt);
}
/*!
    Loads serialized modification data from a variant \a list.
*/
void ParticleModificator::loadData(const VariantList &list) {
    auto it = list.begin();
    m_type = static_cast<ValueType>((*it).toInt());
    it++;
    if(m_type < CURVE) {
        m_min = (*it).toVector4();
        it++;
    }
    if(m_type == RANGE) {
        m_max = (*it).toVector4();
    }
}

/*!
    \class ParticleEmitter
    \brief The ParticleEmitter class represents an emitter for particle systems, providing parameters and settings to control the appearance and behavior of emitted particles.
    \inmodule Resources

    The ParticleEmitter class encapsulates various parameters and settings for controlling the appearance and behavior of particles emitted by a particle system.
    Users can customize these settings, such as associating a mesh and material, defining distribution factors, enabling GPU simulation, setting local or world space, specifying continuous or discrete emission, and applying particle modifiers.
*/

ParticleData::ParticleData() :
        color(1.0f),
        colrate(0.0f),
        transform(0.0f),
        angle(0.0f),
        size(1.0f),
        position(0.0f),
        velocity(0.0f),
        anglerate(0.0f),
        sizerate(0.0f),
        life(1.0f),
        frame(0.0f),
        distance(-1.0f) {

}

class Lifetime: public ParticleModificator {
public:
    void spawnParticle(ParticleData &data) {
        switch(m_type) {
            case CONSTANT: {
                data.life = m_min.x;
            } break;
            case RANGE: {
                data.life = RANGE(m_min.x, m_max.x);
            } break;
            default: break;
        }
    }
};

class StartSize: public ParticleModificator {
public:
    void spawnParticle(ParticleData &data) {
        switch(m_type) {
            case CONSTANT: {
                data.size.x = m_min.x;
                data.size.y = m_min.y;
                data.size.z = m_min.z;
            } break;
            case RANGE: {
                data.size.x = RANGE(m_min.x, m_max.x);
                data.size.y = RANGE(m_min.y, m_max.y);
                data.size.z = RANGE(m_min.z, m_max.z);
            } break;
            default: break;
        }
    }
};

class StartColor: public ParticleModificator {
public:
    void spawnParticle(ParticleData &data) {
        switch(m_type) {
            case CONSTANT: {
                data.color.x = m_min.x;
                data.color.y = m_min.y;
                data.color.z = m_min.z;
                data.color.w = m_min.w;
            } break;
            case RANGE: {
                data.color.x = RANGE(m_min.x, m_max.x);
                data.color.y = RANGE(m_min.y, m_max.y);
                data.color.z = RANGE(m_min.z, m_max.z);
                data.color.w = RANGE(m_min.w, m_max.w);
            } break;
            default: break;
        }
    }
};

class StartAngle: public ParticleModificator {
public:
    void spawnParticle(ParticleData &data) {
        switch(m_type) {
            case CONSTANT: {
                data.angle.x = m_min.x * DEG2RAD;
                data.angle.y = m_min.y * DEG2RAD;
                data.angle.z = m_min.z * DEG2RAD;
            } break;
            case RANGE: {
                data.angle.x = RANGE(m_min.x, m_max.x) * DEG2RAD;
                data.angle.y = RANGE(m_min.y, m_max.y) * DEG2RAD;
                data.angle.z = RANGE(m_min.z, m_max.z) * DEG2RAD;
            } break;
            default: break;
        }
    }
};

class StartPosition: public ParticleModificator {
public:
    void spawnParticle(ParticleData &data) {
        switch(m_type) {
            case CONSTANT: {
                data.position.x = m_min.x;
                data.position.y = m_min.y;
                data.position.z = m_min.z;
            } break;
            case RANGE: {
                data.position.x = RANGE(m_min.x, m_max.x);
                data.position.y = RANGE(m_min.y, m_max.y);
                data.position.z = RANGE(m_min.z, m_max.z);
            } break;
            default: break;
        }
    }
};


class ScaleSize: public ParticleModificator {
public:
    void spawnParticle(ParticleData &data) {
        switch(m_type) {
            case CONSTANT: {
                data.sizerate.x = m_min.x;
                data.sizerate.y = m_min.y;
                data.sizerate.z = m_min.z;
            } break;
            case RANGE: {
                data.sizerate.x = RANGE(m_min.x, m_max.x);
                data.sizerate.y = RANGE(m_min.y, m_max.y);
                data.sizerate.z = RANGE(m_min.z, m_max.z);
            } break;
            default: break;
        }
    }

    void updateParticle(ParticleData &data, float dt) {
        data.size += data.sizerate * dt;
    }
};

class ScaleColor: public ParticleModificator {
public:
    void spawnParticle(ParticleData &data) {
        switch(m_type) {
            case CONSTANT: {
                data.colrate.x = m_min.x;
                data.colrate.y = m_min.y;
                data.colrate.z = m_min.z;
                data.colrate.w = m_min.w;
            } break;
            case RANGE: {
                data.colrate.x = RANGE(m_min.x, m_max.x);
                data.colrate.y = RANGE(m_min.y, m_max.y);
                data.colrate.z = RANGE(m_min.z, m_max.z);
                data.colrate.w = RANGE(m_min.w, m_max.w);
            } break;
            default: break;
        }
    }

    void updateParticle(ParticleData &data, float dt) {
        data.color += data.colrate * dt;
    }
};

class ScaleAngle: public ParticleModificator {
public:
    void spawnParticle(ParticleData &data) {
        switch(m_type) {
            case CONSTANT: {
                data.anglerate.x = m_min.x;
                data.anglerate.y = m_min.y;
                data.anglerate.z = m_min.z;
            } break;
            case RANGE: {
                data.anglerate.x = RANGE(m_min.x, m_max.x);
                data.anglerate.y = RANGE(m_min.y, m_max.y);
                data.anglerate.z = RANGE(m_min.z, m_max.z);
            } break;
            default: break;
        }
    }

    void updateParticle(ParticleData &data, float dt) {
        data.angle += data.anglerate * DEG2RAD * dt;
    }
};

class Velocity: public ParticleModificator {
public:
    void spawnParticle(ParticleData &data) {
        switch(m_type) {
            case CONSTANT: {
                data.velocity.x = m_min.x;
                data.velocity.y = m_min.y;
                data.velocity.z = m_min.z;
            } break;
            case RANGE: {
                data.velocity.x = RANGE(m_min.x, m_max.x);
                data.velocity.y = RANGE(m_min.y, m_max.y);
                data.velocity.z = RANGE(m_min.z, m_max.z);
            } break;
            default: break;
        }
    }

    void updateParticle(ParticleData &data, float dt) {
        data.position += data.velocity * dt;
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
        m_gpu(false),
        m_local(false),
        m_continous(true) {
    PROFILE_FUNCTION();

}

ParticleEffect::~ParticleEffect() {
    PROFILE_FUNCTION();
}
/*!
    Getter for the mesh associated with the particle emitter.
*/
Mesh *ParticleEffect::mesh() const {
    return m_mesh;
}
/*!
    Setter for the \a mesh associated with the particle emitter.
*/
void ParticleEffect::setMesh(Mesh *mesh) {
    m_mesh = mesh;
}
/*!
    Getter for the material associated with the particle emitter.
*/
Material *ParticleEffect::material() const {
    return m_material;
}
/*!
    Setter for the \a material associated with the particle emitter.
*/
void ParticleEffect::setMaterial(Material *material) {
    m_material = material;
}
/*!
    Getter for the distribution factor of emitted particles.
*/
float ParticleEffect::distribution() const {
    return m_distibution;
}
/*!
    Setter for the \a distribution factor of emitted particles.
*/
void ParticleEffect::setDistribution(float distribution) {
    m_distibution = distribution;
}
/*!
    Getter for the local flag indicating local particle space.
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
    Getter for the GPU flag indicating GPU particle simulation.
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
    Getter for the continuous flag indicating continuous particle emission.
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
    Getter for the deque of particle modifiers.
*/
ModifiersDeque &ParticleEffect::modifiers() {
    return m_modifiers;
}
/*!
    Setter for the deque of particle \a modifiers.
*/
void ParticleEffect::setModifiers(const ModifiersDeque &modifiers) {
    m_modifiers = modifiers;
}
/*!
    \internal
*/
void ParticleEffect::loadUserData(const VariantMap &data) {
    PROFILE_FUNCTION();

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
            setDistribution((*it).toFloat());
            it++;

            for(auto &m : (*it).value<VariantList>()) {
                VariantList mods = m.value<VariantList>();
                auto mod = mods.begin();
                int32_t type = (*mod).toInt();

                ParticleModificator *modificator = nullptr;
                switch (type) {
                    case ParticleModificator::LIFETIME:      modificator = new Lifetime(); break;
                    case ParticleModificator::STARTSIZE:     modificator = new StartSize(); break;
                    case ParticleModificator::STARTCOLOR:    modificator = new StartColor(); break;
                    case ParticleModificator::STARTANGLE:    modificator = new StartAngle(); break;
                    case ParticleModificator::STARTPOSITION: modificator = new StartPosition(); break;

                    case ParticleModificator::SCALESIZE:     modificator = new ScaleSize(); break;
                    case ParticleModificator::SCALECOLOR:    modificator = new ScaleColor(); break;
                    case ParticleModificator::SCALEANGLE:    modificator = new ScaleAngle(); break;
                    case ParticleModificator::VELOCITY:      modificator = new Velocity(); break;
                    default: break;
                }

                if(modificator) {
                    mod++;
                    modificator->loadData((*mod).value<VariantList>());
                    m_modifiers.push_back(modificator);
                }
            }
        }
    }
}
