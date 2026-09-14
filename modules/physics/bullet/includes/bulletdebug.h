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
#ifndef BULLETDEBUG_H
#define BULLETDEBUG_H

#ifdef SHARED_DEFINE
#include <list>

#include <btBulletCollisionCommon.h>

#include <amath.h>

class BulletDebug : public btIDebugDraw {
public:
    void setDebugMode(int debugMode) override;

    int getDebugMode() const override;

private:
    void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override;

    void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) override;

    void draw3dText(const btVector3& location, const char* textString) override;

    void reportErrorWarning(const char* warningString) override;

    void clearLines() override;
    void flushLines() override;

private:
    int m_debugMode;

    std::list<Vector3> m_points;
    std::list<uint32_t> m_indices;

};

#endif

#endif // BULLETDEBUG_H
