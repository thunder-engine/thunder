#include "bullet.h"

#include "bulletsystem.h"

#include <cstring>

#ifdef NEXT_SHARED
#include "converters/physicmaterialconverter.h"

Module *moduleCreate(Engine *engine) {
    return new Bullet(engine);
}
#endif

static const char *meta = \
"{"
"   \"version\": \"1.0\","
"   \"module\": \"Bullet\","
"   \"description\": \"BulletPhysics Module\","
"   \"objects\": {"
"       \"BulletSystem\": \"system\","
"       \"PhysicMaterialConverter\": \"converter\""
"   },"
"   \"components\": ["
"       \"BoxCollider\","
"       \"CapsuleCollider\","
"       \"Collider\","
"       \"RigidBody\","
"       \"SphereCollider\","
"       \"VolumeCollider\""
"   ]"
"}";

Bullet::Bullet(Engine *engine) :
        Module(engine),
        m_pSystem(new BulletSystem(engine)) {
}

Bullet::~Bullet() {
    delete m_pSystem;
}

const char *Bullet::metaInfo() const {
    return meta;
}

void *Bullet::getObject(const char *name) {
    if(strcmp(name, "BulletSystem") == 0) {
        return m_pSystem;
    }
#ifdef NEXT_SHARED
    else if(strcmp(name, "PhysicMaterialConverter") == 0) {
        return new PhysicMaterialConverter();
    }
#endif
    return nullptr;
}
