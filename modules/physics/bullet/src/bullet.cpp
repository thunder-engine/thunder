#include "bullet.h"

#include "bulletsystem.h"

#ifdef NEXT_SHARED
#include "converters/physicmaterialconverter.h"

Module *moduleCreate(Engine *engine) {
    return new Bullet(engine);
}
#endif

static const char *meta = \
"{"
"   \"version\": \"1.0\","
"   \"description\": \"BulletPhysics Module\","
"   \"systems\": ["
"       \"BulletSystem\""
"   ],"
"   \"converters\": ["
"       \"PhysicMaterialConverter\""
"   ]"
"}";

Bullet::Bullet(Engine *engine) :
        m_pEngine(engine),
        m_pSystem(new BulletSystem(engine)) {
}

Bullet::~Bullet() {
    delete m_pSystem;
}

const char *Bullet::metaInfo() const {
    return meta;
}

System *Bullet::system(const char *) {
    return m_pSystem;
}
#ifdef NEXT_SHARED
IConverter *Bullet::converter(const char *) {
    return new PhysicMaterialConverter();
}
#endif
