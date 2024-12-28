#include "visualeffect.h"

#include "material.h"
#include "mesh.h"

#include <array>

namespace {
    const char *gEmitters("Emitters");
}

enum Stages {
    State,
    Spawn,
    Update,
    Render
};

enum Operation {
    Mov = 0,
    Add,
    Sub,
    Mul,
    Div
};

enum Space {
    System,
    Emitter,
    Particle,
    Renderable,
    Local,
    Constant,
    Random
};

/*!
    \class VisualEffect
    \brief Contains all necessary information about the effect.
    \inmodule Resources

    VisualEffect alows developer to create a complex visual effects.
*/

VisualEffect::VisualEffect() :
        m_mesh(nullptr),
        m_material(nullptr),
        m_spawnRate(1.0f),
        m_capacity(1),
        m_particleStride(1), // Store an age at least
        m_renderableStride(20), // Matrix4 + Color
        m_gpu(false),
        m_local(false),
        m_continous(true) {

}

void VisualEffect::update(std::vector<float> &emitter, std::vector<float> &particles, std::vector<float> &render) {
    if(!m_spawnOperations.empty()) {
        apply(m_spawnOperations, emitter, particles, render, Spawn);
    }

    if(!m_updateOperations.empty()) {
        apply(m_updateOperations, emitter, particles, render, Update);
    }

    if(!m_renderOperations.empty()) {
        apply(m_renderOperations, emitter, particles, render, Render);
    }
}
/*!
    Returns a mesh associated with the particle emitter.
*/
Mesh *VisualEffect::mesh() const {
    return m_mesh;
}
/*!
    Sets a \a mesh associated with the particle emitter.
*/
void VisualEffect::setMesh(Mesh *mesh) {
    m_mesh = mesh;
}
/*!
    Returns a material associated with the particle emitter.
*/
Material *VisualEffect::material() const {
    return m_material;
}
/*!
    Sets a \a material associated with the particle emitter.
*/
void VisualEffect::setMaterial(Material *material) {
    m_material = material;
}
/*!
    Returns a distribution factor of emitted particles.
*/
float VisualEffect::spawnRate() const {
    return m_spawnRate;
}
/*!
    Sets spawn \a rate factor of emitted particles.
*/
void VisualEffect::setSpawnRate(float rate) {
    m_spawnRate = rate;
}
/*!
    Returns a maximum number of particles to emit.
*/
int VisualEffect::capacity() const {
    return m_capacity;
}
/*!
    Return a size for particle atribute structure.
*/
int VisualEffect::particleStride() const {
    return m_particleStride;
}
/*!
    Return a size for particle atribute structure.
*/
int VisualEffect::renderableStride() const {
    return m_renderableStride;
}
/*!
    Sets a maximum \a capacity of particles to emit.
*/
void VisualEffect::setCapacity(int capacity) {
    m_capacity = capacity;
}
/*!
    \fn bool VisualEffect::local() const

    Returns true if particles are in local space, false otherwise.
*/
bool VisualEffect::local() const {
    return m_local;
}
/*!
    Setter for the \a local flag indicating local particle space.
*/
void VisualEffect::setLocal(bool local) {
    m_local = local;
}
/*!
    Returns true if GPU particle simulation is enabled, false otherwise.

    \note Gpu simulation is not supported yet.
*/
bool VisualEffect::gpu() const {
    return m_gpu;
}
/*!
    Setter for the \a gpu flag indicating GPU particle simulation.

    \note Gpu simulation is not supported yet.
*/
void VisualEffect::setGpu(bool gpu) {
    m_gpu = gpu;
}
/*!
    Returns true for continuous emission, false for one time emission.
*/
bool VisualEffect::continous() const {
    return m_continous;
}
/*!
    Setter for the \a continuous flag indicating continuous particle emission.
*/
void VisualEffect::setContinous(bool continuous) {
    m_continous = continuous;
}
/*!
    Returns bounding box for the emitter.
*/
AABBox VisualEffect::bound() const {
    return m_aabb;
}
/*!
    Modifies a \a particle attributes stored in data buffer.
    The \a emitter buffer contains actual emitter state.
    The \a render buffer contains actual renderables data.
*/
void VisualEffect::apply(std::vector<Operator> &operations, std::vector<float> &emitter, std::vector<float> &particle, std::vector<float> &render, int stage) const {
    Vector4 local;

    bool spawn = stage == Spawn;

    emitter[AliveParticles] = 0;
    for(int i = 0; i < m_capacity; i++) {
        float *p = &particle[i * m_particleStride];
        float *r = &render[static_cast<int32_t>(emitter[AliveParticles]) * m_renderableStride];

        if((*p > 0) != spawn) { // check alive (for spawn case)
            emitter[AliveParticles]++;

            if(spawn) {
                if(emitter[SpawnCounter] < 1.0f) {
                    break;
                }
                emitter[SpawnCounter] -= 1.0f;
            }

            for(auto &it : operations) {
                float *ret = nullptr;
                switch(it.resultSpace) {
                    case Space::Emitter: ret = &emitter[it.resultOffset]; break;
                    case Space::Particle: ret = &p[it.resultOffset]; break;
                    case Space::Renderable: ret = &r[it.resultOffset]; break;
                    case Space::Local: ret = local.v; break;
                    default: break;
                }

                std::array<const float *, 2> arg = {ret, ret};
                std::array<bool, 2> sgl = {false, false};

                for(int b = 0; b < 2; b++) {
                    Argument &argument = it.arguments[b];

                    if(argument.size == 1) {
                        sgl[b] = true;
                    }

                    switch(argument.space) {
                        case Space::Emitter: arg[b] = &emitter[argument.offset]; break;
                        case Space::Particle: arg[b] = &p[argument.offset]; break;
                        case Space::Local: arg[b] = local.v; break;
                        case Space::Constant: arg[b] = it.constData.data(); break;
                        case Space::Random: arg[b] = &it.constData[i * it.resultSize]; break;
                        default: break;
                    }
                }

                for(int c = 0; c < it.resultSize; c++) {
                    float result = 0.0f;
                    float a0 = arg[0][sgl[0] ? 0 : c];
                    float a1 = arg[1][sgl[1] ? 0 : c];

                    switch(it.op) {
                        case Mov: result = a0; break;
                        case Add: result = a0 + a1; break;
                        case Sub: result = a0 - a1; break;
                        case Mul: result = a0 * a1; break;
                        case Div: result = a0 / a1; break;
                        default: break;
                    }

                    ret[c] = result;
                }
            }
        }
    }
}
/*!
    \internal
*/
void VisualEffect::loadUserData(const VariantMap &data) {
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
            setSpawnRate((*it).toFloat());
            it++;
            m_particleStride = (*it).toInt();
            it++;

            loadOperations((*it).value<VariantList>(), m_spawnOperations);
            it++;
            loadOperations((*it).value<VariantList>(), m_updateOperations);
            it++;
            loadOperations((*it).value<VariantList>(), m_renderOperations);
        }
    }
}
/*!
    \internal
*/
void VisualEffect::loadOperations(const VariantList &list, std::vector<Operator> &operations) {
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

        for(Variant arg : (*field).value<VariantList>()) {
            VariantList argFields = arg.value<VariantList>();

            auto argField = argFields.begin();

            Argument argument;

            argument.space = (*argField).toInt();
            argField++;
            argument.size = (*argField).toInt();
            argField++;
            argument.offset = -1;
            switch(argument.space) {
                case Constant: {
                    op.constData.resize(op.resultSize);

                    for(int i = 0; i < op.resultSize; i++) {
                        op.constData[i] = (*argField).toFloat();
                        argField++;
                    }
                } break;
                case Random: {
                    Vector4 min;
                    for(int i = 0; i < op.resultSize; i++) {
                        min[i] = (*argField).toFloat();
                        argField++;
                    }

                    Vector4 max;
                    for(int i = 0; i < op.resultSize; i++) {
                        max[i] = (*argField).toFloat();
                        argField++;
                    }

                    op.constData.resize(m_capacity * op.resultSize);
                    for(int p = 0; p < m_capacity; p++) {
                        for(int i = 0; i < op.resultSize; i++) {
                            op.constData[p * op.resultSize + i] = RANGE(min[i], max[i]);
                        }
                    }
                } break;
                default: {
                    argument.offset = (*argField).toInt();
                } break;
            }

            op.arguments.push_back(argument);
        }

        operations.push_back(op);
    }
}
