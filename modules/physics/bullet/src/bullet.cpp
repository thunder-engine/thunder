/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
#include "bullet.h"

#include "bulletsystem.h"

#include <cstring>

#ifdef SHARED_DEFINE
#include "converters/physicmaterialconverter.h"

Module *moduleCreate(Engine *engine) {
    return new Bullet(engine);
}
#endif

static const char *meta = \
"{"
"   \"module\": \"Bullet\","
"   \"version\": \"1.0\","
"   \"description\": \"Bullet Physics Module\","
"   \"author\": \"Evgeniy Prikazchikov\","
"   \"dependencies\": {"
"       \"bullet3\": \"static\","
"       \"engine\": \"lib\","
"       \"next\": \"lib\""
"   },"
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
"       \"VolumeCollider\","
"       \"CharacterController\","
"       \"MeshCollider\","
"       \"Joint\","
"       \"SpringJoint\","
"       \"HingeJoint\","
"       \"FixedJoint\","
"   ]"
"}";

/*!
    \module Bullet

    \title Bullet physics module for Thunder Engine

    \brief Contains classes related to physics simulation.
*/

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
#ifdef SHARED_DEFINE
    else if(strcmp(name, "PhysicMaterialConverter") == 0) {
        return new PhysicMaterialConverter();
    }
#endif
    return nullptr;
}
