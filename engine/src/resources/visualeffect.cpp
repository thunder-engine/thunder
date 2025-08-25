#include "visualeffect.h"

#include "material.h"
#include "mesh.h"

#include <array>

namespace {
    const char *gEmitters("Emitters");
}

enum Stages {
    EmitterSpawn,
    EmitterUpdate,
    ParticleSpawn,
    ParticleUpdate,
    Render
};

enum Operation {
    Mov = 0,
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    Min,
    Max,
    Floor,
    Ceil,
    Make
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
        m_capacity(1),
        m_systemStride(1),
        m_emitterStride(1),
        m_particleStride(1),
        m_gpu(false),
        m_local(false),
        m_continous(true) {

}

void VisualEffect::update(Buffers &buffers) {
    if(!m_emitterUpdateOperations.empty()) {
        apply(m_emitterUpdateOperations, buffers);
    }

    if(!m_particleSpawnOperations.empty()) {
        for(int p = 0; p < m_capacity; p++) {
            if(buffers.emitter[SpawnCounter] >= 1.0f) {
                float &particleAge = buffers.particles[p * m_particleStride];
                if(particleAge <= 0.0f) {
                    apply(m_particleSpawnOperations, buffers, p);

                    buffers.emitter[SpawnCounter] -= 1.0f;
                }
            } else break;
        }
    }

    if(!m_renderOperations.empty()) {
        buffers.instances = 0;

        for(int r = 0; r < buffers.render.size(); r++) {
            if(m_renderables[r].material) {
                int renderableStride = m_renderables[r].material->uniformSize() / sizeof(float);

                for(int p = 0; p < m_capacity; p++) {
                    float particleAge = buffers.particles[p * m_particleStride];
                    if(particleAge > 0.0f) {
                        apply(m_renderOperations, buffers, p, r, renderableStride);
                        buffers.instances++;
                    }
                }
            }
        }
    }

    if(!m_particleUpdateOperations.empty()) {
        for(int p = 0; p < m_capacity; p++) {
            float particleAge = buffers.particles[p * m_particleStride];
            if(particleAge > 0.0f) {
                apply(m_particleUpdateOperations, buffers, p);
            }
        }
    }
}
/*!
    Returns renderables count.
*/
int VisualEffect::renderablesCount() {
    return m_renderables.size();
}
/*!
    Returns renderable parameters with \a index associated with the particle emitter.
*/
const VisualEffect::Renderable *VisualEffect::renderable(int index) const {
    if(index < m_renderables.size()) {
        return &m_renderables[index];
    }

    return nullptr;
}
/*!
    Returns a maximum number of particles to emit.
*/
int VisualEffect::capacity() const {
    return m_capacity;
}
/*!
    Return a size for system atributes structure.
*/
int VisualEffect::systemStride() const {
    return m_systemStride;
}
/*!
    Return a size for emitter atributes structure.
*/
int VisualEffect::emitterStride() const {
    return m_emitterStride;
}
/*!
    Return a size for particle atributes structure.
*/
int VisualEffect::particleStride() const {
    return m_particleStride;
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

inline void movOp(float *ret, int retSize, const float *arg, int argSize) {
    for(int i = 0; i < retSize; i++) {
        if(i < argSize) {
            ret[i] = arg[i];
        }
    }
}

inline void addOp(float *ret, int retSize, std::array<const float *, 3> &arg, std::array<int, 3> &argSize) {
    for(int i = 0; i < retSize; i++) {
        if(i < argSize[0] && i < argSize[1]) {
            ret[i] = arg[0][i] + arg[1][i];
        }
    }
}

inline void subOp(float *ret, int retSize, std::array<const float *, 3> &arg, std::array<int, 3> &argSize) {
    for(int i = 0; i < retSize; i++) {
        if(i < argSize[0] && i < argSize[1]) {
            ret[i] = arg[0][i] - arg[1][i];
        }
    }
}

inline void mulOp(float *ret, int retSize, std::array<const float *, 3> &arg, std::array<int, 3> &argSize) {
    if(argSize[0] == 16) {
        const Matrix4 *m = reinterpret_cast<const Matrix4 *>(arg[0]);
        const Vector3 *v = reinterpret_cast<const Vector3 *>(arg[1]);

        Vector3 *r = reinterpret_cast<Vector3 *>(ret);

        *r = *m * *v;
    } else {
        for(int i = 0; i < retSize; i++) {
            if(i < argSize[0] && i < argSize[1]) {
                ret[i] = arg[0][i] * arg[1][i];
            }
        }
    }
}

inline void divOp(float *ret, int retSize, std::array<const float *, 3> &arg, std::array<int, 3> &argSize) {
    for(int i = 0; i < retSize; i++) {
        if(i < argSize[0] && i < argSize[1]) {
            ret[i] = arg[0][i] / arg[1][i];
        }
    }
}

inline void modOp(float *ret, int retSize, std::array<const float *, 3> &arg, std::array<int, 3> &argSize) {
    for(int i = 0; i < retSize; i++) {
        if(i < argSize[0] && i < argSize[1]) {
            float y = arg[1][i];
            ret[i] = modf(arg[0][i], &y);
        }
    }
}

inline void minOp(float *ret, int retSize, std::array<const float *, 3> &arg, std::array<int, 3> &argSize) {
    for(int i = 0; i < retSize; i++) {
        if(i < argSize[0] && i < argSize[1]) {
            ret[i] = MIN(arg[0][i], arg[1][i]);
        }
    }
}

inline void maxOp(float *ret, int retSize, std::array<const float *, 3> &arg, std::array<int, 3> &argSize) {
    for(int i = 0; i < retSize; i++) {
        if(i < argSize[0] && i < argSize[1]) {
            ret[i] = MAX(arg[0][i], arg[1][i]);
        }
    }
}

inline void floorOp(float *ret, int retSize, const float *arg, int argSize) {
    for(int i = 0; i < retSize; i++) {
        if(i < argSize) {
            ret[i] = floor(arg[i]);
        }
    }
}

inline void ceilOp(float *ret, int retSize, const float *arg, int argSize) {
    for(int i = 0; i < retSize; i++) {
        if(i < argSize) {
            ret[i] = ceil(arg[i]);
        }
    }
}

inline void makeOp(float *ret, int retSize, std::array<const float *, 3> &arg, std::array<int, 3> &argSize) {
    if(retSize == 16) {
        const Vector3 *t = reinterpret_cast<const Vector3 *>(arg[0]);
        const Vector3 *r = reinterpret_cast<const Vector3 *>(arg[1]);
        const Vector3 *s = reinterpret_cast<const Vector3 *>(arg[2]);

        *reinterpret_cast<Matrix4 *>(ret) = Matrix4(*t, Quaternion(*r), *s);
    }
}

/*!
    \internal
*/
void VisualEffect::apply(std::vector<Operator> &operations, Buffers &buffers, int particle, int render, int stride) const {
    Vector4 local;

    float *p = &buffers.particles[particle * m_particleStride];

     for(auto &it : operations) {
        float *ret = nullptr;
        switch(it.resultSpace) {
            case Space::System: ret = &buffers.system[it.resultOffset]; break;
            case Space::Emitter: ret = &buffers.emitter[it.resultOffset]; break;
            case Space::Particle: ret = &p[it.resultOffset]; break;
            case Space::Renderable: ret = &buffers.render[render][buffers.instances * stride + it.resultOffset]; break;
            case Space::Local: ret = local.v; break;
            default: break;
        }

        std::array<const float *, 3> arg = {ret, ret, ret};
        std::array<int, 3> size = {1, 1, 1};

        for(int a = 0; a < it.arguments.size(); a++) {
            const Argument &argument = it.arguments[a];
            size[a] = argument.size;

            switch(argument.space) {
                case Space::System: arg[a] = &buffers.system[argument.offset]; break;
                case Space::Emitter: arg[a] = &buffers.emitter[argument.offset]; break;
                case Space::Particle: arg[a] = &p[argument.offset]; break;
                case Space::Local: arg[a] = local.v; break;
                case Space::Constant: arg[a] = &it.constData[argument.offset]; break;
                case Space::Random: arg[a] = &it.constData[particle * it.resultSize + argument.offset]; break;
                default: break;
            }
        }

        switch(it.op) {
            case Mov: movOp(ret, it.resultSize, arg[0], size[0]); break;
            case Add: addOp(ret, it.resultSize, arg, size); break;
            case Sub: subOp(ret, it.resultSize, arg, size); break;
            case Mul: mulOp(ret, it.resultSize, arg, size); break;
            case Div: divOp(ret, it.resultSize, arg, size); break;
            case Mod: modOp(ret, it.resultSize, arg, size); break;
            case Min: minOp(ret, it.resultSize, arg, size); break;
            case Max: maxOp(ret, it.resultSize, arg, size); break;
            case Floor: floorOp(ret, it.resultSize, arg[0], size[0]); break;
            case Ceil: ceilOp(ret, it.resultSize, arg[0], size[0]); break;
            case Make: makeOp(ret, it.resultSize, arg, size); break;
            default: break;
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

            setGpu((*it).toBool());
            it++;
            setLocal((*it).toBool());
            it++;
            setContinous((*it).toBool());
            it++;
            setCapacity((*it).toInt());
            it++;
            m_systemStride = (*it).toInt();
            it++;
            m_emitterStride = (*it).toInt();
            it++;
            m_particleStride = (*it).toInt();
            it++;

            loadRenderables((*it).value<VariantList>());
            it++;

            loadOperations((*it).value<VariantList>(), m_emitterSpawnOperations);
            it++;
            loadOperations((*it).value<VariantList>(), m_emitterUpdateOperations);
            it++;
            loadOperations((*it).value<VariantList>(), m_particleSpawnOperations);
            it++;
            loadOperations((*it).value<VariantList>(), m_particleUpdateOperations);
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
        op.resultSize = (*field).toInt();
        field++;
        op.resultOffset = (*field).toInt();
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
                    argument.offset = op.constData.size();
                    op.constData.resize(argument.offset + argument.size);

                    for(int i = 0; i < argument.size; i++) {
                        op.constData[argument.offset + i] = (*argField).toFloat();
                        argField++;
                    }
                } break;
                case Random: {
                    Vector4 min;
                    for(int i = 0; i < argument.size; i++) {
                        min[i] = (*argField).toFloat();
                        argField++;
                    }

                    Vector4 max;
                    for(int i = 0; i < argument.size; i++) {
                        max[i] = (*argField).toFloat();
                        argField++;
                    }

                    argument.offset = op.constData.size();
                    op.constData.resize(argument.offset + m_capacity * argument.size);
                    for(int p = 0; p < m_capacity; p++) {
                        for(int i = 0; i < argument.size; i++) {
                            op.constData[p * argument.size + argument.offset + i] = RANGE(min[i], max[i]);
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
/*!
    \internal
*/
void VisualEffect::loadRenderables(const VariantList &list) {
    for(auto it : m_renderables) {
        it.material->decRef();
        it.mesh->decRef();
    }
    m_renderables.clear();

    for(auto it : list) {
        VariantList renderableFields = it.value<VariantList>();

        auto field = renderableFields.begin();
        Material::SurfaceType type = static_cast<Material::SurfaceType>((*field).toInt());
        field++;
        Mesh *mesh = Engine::loadResource<Mesh>((*field).toString());
        field++;
        Material *material = Engine::loadResource<Material>((*field).toString());
        field++;

        if(mesh && material) {
            mesh->incRef();
            material->incRef();

            m_renderables.push_back({type, mesh, material});
        }
    }
}
