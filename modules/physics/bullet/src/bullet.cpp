#include "bullet.h"

#include "bulletsystem.h"

#ifdef NEXT_SHARED
    #include "converters/physicmaterialconverter.h"
#endif
IModule *moduleCreate(Engine *engine) {
    return new Bullet(engine);
}

Bullet::Bullet(Engine *engine) :
        m_pEngine(engine),
        m_pSystem(new BulletSystem(engine)) {
}

Bullet::~Bullet() {
    delete m_pSystem;
}

const char *Bullet::description() const {
    return "BulletPhysics Module";
}

const char *Bullet::version() const {
    return "1.0";
}

uint8_t Bullet::types() const {
    uint8_t result  = SYSTEM;
#ifdef NEXT_SHARED
    result  |= CONVERTER;
#endif
    return result;
}

ISystem *Bullet::system() {
    return m_pSystem;
}

IConverter *Bullet::converter() {
#ifdef NEXT_SHARED
    return new PhysicMaterialConverter();
#endif
    return nullptr;
}
