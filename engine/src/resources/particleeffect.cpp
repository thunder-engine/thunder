#include "particleeffect.h"

#include "material.h"
#include "mesh.h"

#define EMITTERS "Emitters"

ParticleModificator::ParticleModificator() :
    m_Type(CONSTANT),
    m_Min(1.0f),
    m_Max(1.0f) {
}

ParticleModificator::~ParticleModificator() {

}

void ParticleModificator::spawnParticle(ParticleData &data) {
    A_UNUSED(data);
}

void ParticleModificator::updateParticle(ParticleData &data, float dt) {
    A_UNUSED(data);
    A_UNUSED(dt);
}

void ParticleModificator::loadData(const VariantList &list) {
    auto it = list.begin();
    m_Type = static_cast<ValueType>((*it).toInt());
    it++;
    if(m_Type < CURVE) {
        m_Min = (*it).toVector4();
        it++;
    }
    if(m_Type == RANGE) {
        m_Max = (*it).toVector4();
    }
}

ParticleEmitter::ParticleEmitter() :
        m_pMesh(nullptr),
        m_pMaterial(nullptr),
        m_Distibution(1.0f),
        m_Gpu(false),
        m_Local(false),
        m_Continous(true) {

}

bool ParticleEmitter::operator== (const ParticleEmitter &emitter) const {
    return (m_pMesh == emitter.m_pMesh) &&
           (m_pMaterial == emitter.m_pMaterial) &&
           (m_Distibution == emitter.m_Distibution) &&
           (m_Gpu == emitter.m_Gpu) &&
           (m_Local == emitter.m_Local) &&
           (m_Continous == emitter.m_Continous);
}

Mesh *ParticleEmitter::mesh() const {
    return m_pMesh;
}
void ParticleEmitter::setMesh(Mesh *mesh) {
    m_pMesh = mesh;
}

Material *ParticleEmitter::material() const {
    return m_pMaterial;
}
void ParticleEmitter::setMaterial(Material *material) {
    m_pMaterial = material;
}

float ParticleEmitter::distibution() const {
    return m_Distibution;
}
void ParticleEmitter::setDistibution(float distibution) {
    m_Distibution = distibution;
}

bool ParticleEmitter::local() const {
    return m_Local;
}
void ParticleEmitter::setLocal(bool local) {
    m_Local = local;
}

bool ParticleEmitter::gpu() const {
    return m_Gpu;
}
void ParticleEmitter::setGpu(bool gpu) {
    m_Gpu = gpu;
}

bool ParticleEmitter::continous() const {
    return m_Continous;
}
void ParticleEmitter::setContinous(bool continous) {
    m_Continous = continous;
}

ModifiersDeque &ParticleEmitter::modifiers() {
    return m_Modifiers;
}
void ParticleEmitter::setModifiers(const ModifiersDeque &modifiers) {
    m_Modifiers = modifiers;
}


ParticleData::ParticleData() :
        life(1.0),
        frame(0.0),
        distance(-1.0f),
        transform(0.0f),
        angle(0.0f),
        color(1.0f),
        size(1.0f),
        colrate(0.0f),
        position(0.0f),
        velocity(0.0f),
        anglerate(0.0f),
        sizerate(0.0f) {

}

class Lifetime: public ParticleModificator {
public:
    void spawnParticle(ParticleData &data) {
        switch(m_Type) {
            case CONSTANT: {
                data.life = m_Min.x;
            } break;
            case RANGE: {
                data.life = RANGE(m_Min.x, m_Max.x);
            } break;
            default: break;
        }
    }
};

class StartSize: public ParticleModificator {
public:
    void spawnParticle(ParticleData &data) {
        switch(m_Type) {
            case CONSTANT: {
                data.size.x = m_Min.x;
                data.size.y = m_Min.y;
                data.size.z = m_Min.z;
            } break;
            case RANGE: {
                data.size.x = RANGE(m_Min.x, m_Max.x);
                data.size.y = RANGE(m_Min.y, m_Max.y);
                data.size.z = RANGE(m_Min.z, m_Max.z);
            } break;
            default: break;
        }
    }
};

class StartColor: public ParticleModificator {
public:
    void spawnParticle(ParticleData &data) {
        switch(m_Type) {
            case CONSTANT: {
                data.color.x = m_Min.x;
                data.color.y = m_Min.y;
                data.color.z = m_Min.z;
                data.color.w = m_Min.w;
            } break;
            case RANGE: {
                data.color.x = RANGE(m_Min.x, m_Max.x);
                data.color.y = RANGE(m_Min.y, m_Max.y);
                data.color.z = RANGE(m_Min.z, m_Max.z);
                data.color.w = RANGE(m_Min.w, m_Max.w);
            } break;
            default: break;
        }
    }
};

class StartAngle: public ParticleModificator {
public:
    void spawnParticle(ParticleData &data) {
        switch(m_Type) {
            case CONSTANT: {
                data.angle.x = m_Min.x * DEG2RAD;
                data.angle.y = m_Min.y * DEG2RAD;
                data.angle.z = m_Min.z * DEG2RAD;
            } break;
            case RANGE: {
                data.angle.x = RANGE(m_Min.x, m_Max.x) * DEG2RAD;
                data.angle.y = RANGE(m_Min.y, m_Max.y) * DEG2RAD;
                data.angle.z = RANGE(m_Min.z, m_Max.z) * DEG2RAD;
            } break;
            default: break;
        }
    }
};

class StartPosition: public ParticleModificator {
public:
    void spawnParticle(ParticleData &data) {
        switch(m_Type) {
            case CONSTANT: {
                data.position.x = m_Min.x;
                data.position.y = m_Min.y;
                data.position.z = m_Min.z;
            } break;
            case RANGE: {
                data.position.x = RANGE(m_Min.x, m_Max.x);
                data.position.y = RANGE(m_Min.y, m_Max.y);
                data.position.z = RANGE(m_Min.z, m_Max.z);
            } break;
            default: break;
        }
    }
};


class ScaleSize: public ParticleModificator {
public:
    void spawnParticle(ParticleData &data) {
        switch(m_Type) {
            case CONSTANT: {
                data.sizerate.x = m_Min.x;
                data.sizerate.y = m_Min.y;
                data.sizerate.z = m_Min.z;
            } break;
            case RANGE: {
                data.sizerate.x = RANGE(m_Min.x, m_Max.x);
                data.sizerate.y = RANGE(m_Min.y, m_Max.y);
                data.sizerate.z = RANGE(m_Min.z, m_Max.z);
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
        switch(m_Type) {
            case CONSTANT: {
                data.colrate.x = m_Min.x;
                data.colrate.y = m_Min.y;
                data.colrate.z = m_Min.z;
                data.colrate.w = m_Min.w;
            } break;
            case RANGE: {
                data.colrate.x = RANGE(m_Min.x, m_Max.x);
                data.colrate.y = RANGE(m_Min.y, m_Max.y);
                data.colrate.z = RANGE(m_Min.z, m_Max.z);
                data.colrate.w = RANGE(m_Min.w, m_Max.w);
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
        switch(m_Type) {
            case CONSTANT: {
                data.anglerate.x = m_Min.x;
                data.anglerate.y = m_Min.y;
                data.anglerate.z = m_Min.z;
            } break;
            case RANGE: {
                data.anglerate.x = RANGE(m_Min.x, m_Max.x);
                data.anglerate.y = RANGE(m_Min.y, m_Max.y);
                data.anglerate.z = RANGE(m_Min.z, m_Max.z);
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
        switch(m_Type) {
            case CONSTANT: {
                data.velocity.x = m_Min.x;
                data.velocity.y = m_Min.y;
                data.velocity.z = m_Min.z;
            } break;
            case RANGE: {
                data.velocity.x = RANGE(m_Min.x, m_Max.x);
                data.velocity.y = RANGE(m_Min.y, m_Max.y);
                data.velocity.z = RANGE(m_Min.z, m_Max.z);
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

ParticleEffect::ParticleEffect() {
    PROFILE_FUNCTION();

}

ParticleEffect::~ParticleEffect() {
    PROFILE_FUNCTION();
}
/*!
    Removes all emitters from the effect
*/
void ParticleEffect::clear() {
    m_Emitters.clear();
}
/*!
    Returns a count of the emitters for effect.
*/
int ParticleEffect::emittersCount() const {
    PROFILE_FUNCTION();
    return m_Emitters.size();
}
/*!
    Returns an emitter with \a index.
*/
ParticleEmitter *ParticleEffect::emitter(int index) {
    PROFILE_FUNCTION();
    return &m_Emitters[index];
}
/*!
    Adds an \a emitter to the effect.
*/
void ParticleEffect::addEmitter(ParticleEmitter *emitter) {
    if(emitter) {
        m_Emitters.push_back(*emitter);
    }
}
/*!
    \internal
*/
void ParticleEffect::loadUserData(const VariantMap &data) {
    PROFILE_FUNCTION();
    clear();
    {
        auto section = data.find(EMITTERS);
        if(section != data.end()) {
            VariantList list = (*section).second.value<VariantList>();
            for(auto e : list) {
                VariantList fields = e.value<VariantList>();
                auto it = fields.begin();
                ParticleEmitter emitter;

                emitter.setMesh(Engine::loadResource<Mesh>((*it).toString()));
                it++;
                emitter.setMaterial(Engine::loadResource<Material>((*it).toString()));
                it++;
                emitter.setGpu((*it).toBool());
                it++;
                emitter.setLocal((*it).toBool());
                it++;
                emitter.setContinous((*it).toBool());
                it++;
                emitter.setDistibution((*it).toFloat());
                it++;

                for(auto m : (*it).value<VariantList>()) {
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
                        emitter.modifiers().push_back(modificator);
                    }
                }

                m_Emitters.push_back(emitter);
            }
        }
    }
}
/*!
    \internal

    \warning Do not call this function manually
*/
void ParticleEffect::registerSuper(ObjectSystem *system) {
    REGISTER_META_TYPE(ParticleEmitter);
    ParticleEffect::registerClassFactory(system);
}
