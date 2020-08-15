#include "particleeffect.h"

#include "material.h"
#include "mesh.h"

#include <anim/variantanimation.h>

#define EMITTERS "Emitters"

ParticleEffect::Emitter::Emitter() :
        m_pMesh(nullptr),
        m_pMaterial(nullptr),
        m_Distibution(1.0f),
        m_Gpu(false),
        m_Local(false),
        m_Continous(true) {

}

ParticleEffect::ParticleData::ParticleData() :
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

class ParticleModificator {
public:
    enum ModificatorType {
        MODIFICATOR_CONSTANT    = 0,
        MODIFICATOR_RANGE,
        MODIFICATOR_CURVE
    };

public:
    ParticleModificator() :
        m_Type(MODIFICATOR_CONSTANT),
        m_Min(1.0f),
        m_Max(1.0f) {
    }

    virtual ~ParticleModificator() {}

    virtual void spawnParticle(ParticleEffect::ParticleData &data) {
        A_UNUSED(data);
    }

    virtual void updateParticle(ParticleEffect::ParticleData &data, float dt) {
        A_UNUSED(data);
        A_UNUSED(dt);
    }

    void loadData(const VariantList &list) {
        auto it = list.begin();
        m_Type = static_cast<ModificatorType>((*it).toInt());
        it++;
        if(m_Type < MODIFICATOR_CURVE) {
            m_Min = (*it).toVector4();
            it++;
        }
        if(m_Type == MODIFICATOR_RANGE) {
            m_Max = (*it).toVector4();
        }
    }

protected:
    ModificatorType m_Type;
    Vector4 m_Min;
    Vector4 m_Max;

    VariantAnimation m_Curve;
};

class Lifetime: public ParticleModificator {
public:
    void spawnParticle(ParticleEffect::ParticleData &data) {
        switch(m_Type) {
            case MODIFICATOR_CONSTANT: {
                data.life = m_Min.x;
            } break;
            case MODIFICATOR_RANGE: {
                data.life = RANGE(m_Min.x, m_Max.x);
            } break;
            default: break;
        }
    }
};

class StartSize: public ParticleModificator {
public:
    void spawnParticle(ParticleEffect::ParticleData &data) {
        switch(m_Type) {
            case MODIFICATOR_CONSTANT: {
                data.size.x = m_Min.x;
                data.size.y = m_Min.y;
                data.size.z = m_Min.z;
            } break;
            case MODIFICATOR_RANGE: {
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
    void spawnParticle(ParticleEffect::ParticleData &data) {
        switch(m_Type) {
            case MODIFICATOR_CONSTANT: {
                data.color.x = m_Min.x;
                data.color.y = m_Min.y;
                data.color.z = m_Min.z;
                data.color.w = m_Min.w;
            } break;
            case MODIFICATOR_RANGE: {
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
    void spawnParticle(ParticleEffect::ParticleData &data) {
        switch(m_Type) {
            case MODIFICATOR_CONSTANT: {
                data.angle.x = m_Min.x * DEG2RAD;
                data.angle.y = m_Min.y * DEG2RAD;
                data.angle.z = m_Min.z * DEG2RAD;
            } break;
            case MODIFICATOR_RANGE: {
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
    void spawnParticle(ParticleEffect::ParticleData &data) {
        switch(m_Type) {
            case MODIFICATOR_CONSTANT: {
                data.position.x = m_Min.x;
                data.position.y = m_Min.y;
                data.position.z = m_Min.z;
            } break;
            case MODIFICATOR_RANGE: {
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
    void spawnParticle(ParticleEffect::ParticleData &data) {
        switch(m_Type) {
            case MODIFICATOR_CONSTANT: {
                data.sizerate.x = m_Min.x;
                data.sizerate.y = m_Min.y;
                data.sizerate.z = m_Min.z;
            } break;
            case MODIFICATOR_RANGE: {
                data.sizerate.x = RANGE(m_Min.x, m_Max.x);
                data.sizerate.y = RANGE(m_Min.y, m_Max.y);
                data.sizerate.z = RANGE(m_Min.z, m_Max.z);
            } break;
            default: break;
        }
    }

    void updateParticle(ParticleEffect::ParticleData &data, float dt) {
        data.size += data.sizerate * dt;
    }
};

class ScaleColor: public ParticleModificator {
public:
    void spawnParticle(ParticleEffect::ParticleData &data) {
        switch(m_Type) {
            case MODIFICATOR_CONSTANT: {
                data.colrate.x = m_Min.x;
                data.colrate.y = m_Min.y;
                data.colrate.z = m_Min.z;
                data.colrate.w = m_Min.w;
            } break;
            case MODIFICATOR_RANGE: {
                data.colrate.x = RANGE(m_Min.x, m_Max.x);
                data.colrate.y = RANGE(m_Min.y, m_Max.y);
                data.colrate.z = RANGE(m_Min.z, m_Max.z);
                data.colrate.w = RANGE(m_Min.w, m_Max.w);
            } break;
            default: break;
        }
    }

    void updateParticle(ParticleEffect::ParticleData &data, float dt) {
        data.color += data.colrate * dt;
    }
};

class ScaleAngle: public ParticleModificator {
public:
    void spawnParticle(ParticleEffect::ParticleData &data) {
        switch(m_Type) {
            case MODIFICATOR_CONSTANT: {
                data.anglerate.x = m_Min.x;
                data.anglerate.y = m_Min.y;
                data.anglerate.z = m_Min.z;
            } break;
            case MODIFICATOR_RANGE: {
                data.anglerate.x = RANGE(m_Min.x, m_Max.x);
                data.anglerate.y = RANGE(m_Min.y, m_Max.y);
                data.anglerate.z = RANGE(m_Min.z, m_Max.z);
            } break;
            default: break;
        }
    }

    void updateParticle(ParticleEffect::ParticleData &data, float dt) {
        data.angle += data.anglerate * DEG2RAD * dt;
    }
};

class Velocity: public ParticleModificator {
public:
    void spawnParticle(ParticleEffect::ParticleData &data) {
        switch(m_Type) {
            case MODIFICATOR_CONSTANT: {
                data.velocity.x = m_Min.x;
                data.velocity.y = m_Min.y;
                data.velocity.z = m_Min.z;
            } break;
            case MODIFICATOR_RANGE: {
                data.velocity.x = RANGE(m_Min.x, m_Max.x);
                data.velocity.y = RANGE(m_Min.y, m_Max.y);
                data.velocity.z = RANGE(m_Min.z, m_Max.z);
            } break;
            default: break;
        }
    }

    void updateParticle(ParticleEffect::ParticleData &data, float dt) {
        data.position += data.velocity * dt;
    }
};


ParticleEffect::ParticleEffect() {
    PROFILE_FUNCTION();

}

ParticleEffect::~ParticleEffect() {
    PROFILE_FUNCTION();
}

void ParticleEffect::clear() {
    m_Emitters.clear();
}

uint32_t ParticleEffect::emittersCount() const {
    PROFILE_FUNCTION();
    return m_Emitters.size();
}

ParticleEffect::Emitter &ParticleEffect::emitter(uint32_t index) {
    PROFILE_FUNCTION();
    return m_Emitters[index];
}

void ParticleEffect::spawnParticle(uint32_t index, ParticleData &data) {
    PROFILE_FUNCTION();
    data.position.x = 0.0f;
    data.position.y = 0.0f;
    data.position.z = 0.0f;
    for(auto it : m_Emitters[index].m_Modifiers) {
        it->spawnParticle(data);
    }
}

void ParticleEffect::updateParticle(uint32_t index, ParticleData &data, float dt) {
    PROFILE_FUNCTION();
    for(auto it : m_Emitters[index].m_Modifiers) {
        it->updateParticle(data, dt);
    }
}

AABBox ParticleEffect::bound() const {
    PROFILE_FUNCTION();
    return AABBox();
}

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
                Emitter emitter;

                emitter.m_pMesh = Engine::loadResource<Mesh>((*it).toString());
                it++;
                emitter.m_pMaterial = Engine::loadResource<Material>((*it).toString());
                it++;
                emitter.m_Gpu = (*it).toBool();
                it++;
                emitter.m_Local = (*it).toBool();
                it++;
                emitter.m_Continous = (*it).toBool();
                it++;
                emitter.m_Distibution = (*it).toFloat();
                it++;

                for(auto m : (*it).value<VariantList>()) {
                    VariantList mods = m.value<VariantList>();
                    auto mod = mods.begin();
                    int32_t type = (*mod).toInt();

                    ParticleModificator *modificator = nullptr;
                    switch (type) {
                        case ParticleEffect::MODIFICATOR_LIFETIME:      modificator = new Lifetime; break;
                        case ParticleEffect::MODIFICATOR_STARTSIZE:     modificator = new StartSize; break;
                        case ParticleEffect::MODIFICATOR_STARTCOLOR:    modificator = new StartColor; break;
                        case ParticleEffect::MODIFICATOR_STARTANGLE:    modificator = new StartAngle; break;
                        case ParticleEffect::MODIFICATOR_STARTPOSITION: modificator = new StartPosition; break;

                        case ParticleEffect::MODIFICATOR_SCALESIZE:     modificator = new ScaleSize; break;
                        case ParticleEffect::MODIFICATOR_SCALECOLOR:    modificator = new ScaleColor; break;
                        case ParticleEffect::MODIFICATOR_SCALEANGLE:    modificator = new ScaleAngle; break;
                        case ParticleEffect::MODIFICATOR_VELOCITY:      modificator = new Velocity; break;
                        default: break;
                    }

                    if(modificator != nullptr) {
                        mod++;
                        modificator->loadData((*mod).value<VariantList>());
                        emitter.m_Modifiers.push_back(modificator);
                    }
                }

                m_Emitters.push_back(emitter);
            }
        }
    }

    setState(Ready);
}
