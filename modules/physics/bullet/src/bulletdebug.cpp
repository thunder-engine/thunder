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
#include "bulletdebug.h"

#ifdef SHARED_DEFINE
#include <gizmos.h>

void BulletDebug::setDebugMode(int debugMode) {
    m_debugMode = debugMode;
}

int BulletDebug::getDebugMode() const {
    return m_debugMode;
}

void BulletDebug::drawLine(const btVector3& from, const btVector3& to, const btVector3& color) {
    m_points.push_back(Vector3(from.x(), from.y(), from.z()));
    m_points.push_back(Vector3(to.x(), to.y(), to.z()));

    m_indices.push_back(m_indices.size());
    m_indices.push_back(m_indices.size());
}

void BulletDebug::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) {

}

void BulletDebug::draw3dText(const btVector3& location, const char* textString) {

}

void BulletDebug::reportErrorWarning(const char* warningString) {

}

void BulletDebug::clearLines() {
    m_points.clear();
    m_indices.clear();
}

void BulletDebug::flushLines() {
    Gizmos::drawLines(Vector3Vector(m_points.begin(), m_points.end()), IndexVector(m_indices.begin(), m_indices.end()), Vector4(1.0f));
}

#endif
