#include "particleeffect.h"

#include "material.h"
#include "mesh.h"

namespace {
    const char *gEmitters("Emitters");
}

enum ParameterMode {
    Value = -1,
    Constant,
    Random
};

enum Operation {
    Set = 0,
    Add,
    Subtract,
    Multiply,
    Divide
};

enum Space {
    System,
    Emitter,
    Particle,
    Renderable
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
        m_spawnRate(1.0f),
        m_capacity(1),
        m_attributeStride(1), // Store an age at least
        m_gpu(false),
        m_local(false),
        m_continous(true) {

}

void ParticleEffect::update(std::vector<float> &emitter, std::vector<float> &particles) {
    apply(m_spawnOperations, emitter, particles, true);

    apply(m_updateOperations, emitter, particles, false);
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
float ParticleEffect::spawnRate() const {
    return m_spawnRate;
}
/*!
    Sets spawn \a rate factor of emitted particles.
*/
void ParticleEffect::setSpawnRate(float rate) {
    m_spawnRate = rate;
}
/*!
    Returns a maximum number of particles to emit.
*/
int ParticleEffect::capacity() const {
    return m_capacity;
}
/*!
    Return a size for particle atribute structure.
*/
inline int ParticleEffect::attributeStride() const {
    return m_attributeStride;
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
    Modifies a \a particle attributes stored in data buffer.
    The \a emitter buffer contains actual emitter state.
*/

/*!
    \internal
*/
void ParticleEffect::loadUserData(const VariantMap &data) {
    PROFILE_FUNCTION();

    Resource::loadUserData(data);

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
            m_attributeStride = (*it).toInt();
            it++;
            setSpawnRate((*it).toFloat());
            it++;

            loadOperations((*it).value<VariantList>(), m_spawnOperations);
            it++;
            loadOperations((*it).value<VariantList>(), m_updateOperations);
            it++;
            loadOperations((*it).value<VariantList>(), m_renderOperations);
        }
    }
}

void ParticleEffect::apply(std::vector<Operator> &operations, std::vector<float> &emitter, std::vector<float> &particle, bool spawn) const {
    for(int i = 0; i < m_capacity; i++) {
        int index = m_attributeStride * i;
        float *p = &particle[index];
        if((*p > 0) != spawn) {
            for(auto &it : operations) {
                float *r = &p[it.resultOffset];

                const float *v = nullptr;
                switch(it.mode) {
                    case Value: {
                        switch(it.argSpace) {
                            case Space::Emitter: {
                                v = &emitter[it.argOffset];
                            } break;
                            case Space::Particle: {
                                v = &p[it.argOffset];
                            } break;
                            default: break;
                        }
                    } break;
                    case Constant: v = it.argData.data(); break;
                    case Random: v = &it.argData[i]; break;
                    default: break;
                }

                switch(it.op) {
                    case Set: {
                        for(int c = 0; c < it.resultSize; c++) {
                            r[c] = v[c];
                        }
                    } break;
                    case Add: {
                        for(int c = 0; c < it.resultSize; c++) {
                            r[c] += v[c];
                        }
                    } break;
                    case Subtract: {
                        for(int c = 0; c < it.resultSize; c++) {
                            r[c] -= v[c];
                        }
                    } break;
                    case Multiply: {
                        for(int c = 0; c < it.resultSize; c++) {
                            r[c] *= v[c];
                        }
                    } break;
                    case Divide: {
                        for(int c = 0; c < it.resultSize; c++) {
                            r[c] /= v[c];
                        }
                    } break;
                    default: break;
                }
            }
        }
    }
}

void ParticleEffect::loadOperations(const VariantList &list, std::vector<Operator> &operations) {
    operations.clear();

    for(auto it : list) {
        VariantList fields = it.value<VariantList>();

        Operator op;
        auto field = fields.begin();
        op.op = static_cast<Operation>((*field).toInt());
        field++;
        op.resultSpace = (*field).toInt();
        field++;
        op.resultOffset = (*field).toInt();
        field++;
        op.resultSize = (*field).toInt();
        field++;
        op.mode = static_cast<ParameterMode>((*field).toInt());
        field++;

        op.argSpace = -1;
        op.argOffset = -1;
        op.argSize = -1;
        switch(op.mode) {
            case Value: {
                op.argSpace = (*field).toInt();
                field++;
                op.argOffset = (*field).toInt();
                field++;
                op.argSize = (*field).toInt();
            } break;
            case Constant: {
                op.argData.resize(op.resultSize);

                for(int i = 0; i < op.resultSize; i++) {
                    op.argData[i] = (*field).toFloat();
                    field++;
                }
            } break;
            case Random: {
                Vector4 min;
                for(int i = 0; i < op.resultSize; i++) {
                    min[i] = (*field).toFloat();
                    field++;
                }

                Vector4 max;
                for(int i = 0; i < op.resultSize; i++) {
                    max[i] = (*field).toFloat();
                    field++;
                }

                op.argData.resize(m_capacity * op.resultSize);
                for(int p = 0; p < m_capacity; p++) {
                    for(int i = 0; i < op.resultSize; i++) {
                        op.argData[p * op.resultSize + i] = RANGE(min[i], max[i]);
                    }
                }
            } break;
            default: break;
        }

        operations.push_back(op);
    }
}
