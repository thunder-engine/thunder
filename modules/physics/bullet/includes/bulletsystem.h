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
#ifndef BULLETSYSTEM_H
#define BULLETSYSTEM_H

#include <system.h>

class Engine;
class Collider;
class Joint;

class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDynamicsWorld;

class BulletSystem : public System {
public:
    BulletSystem(Engine *engine);
    ~BulletSystem() override;

private:
    void update(World *world) override;

    int threadPolicy() const override;

    void addObject(Object *object) override;

    void removeObject(Object *object) override;

    static bool rayCast(System *system, World *world, const Ray &ray, float distance, Ray::Hit *hit);

protected:
    std::unordered_map<uint32_t, btDynamicsWorld *> m_worlds;

    std::list<Collider *> m_colliderList;

    std::list<Joint *> m_jointList;

    btDefaultCollisionConfiguration *m_collisionConfiguration;

    btCollisionDispatcher *m_dispatcher;

    btBroadphaseInterface *m_overlappingPairCache;

    btSequentialImpulseConstraintSolver *m_solver;

};

#endif // BULLETSYSTEM_H
