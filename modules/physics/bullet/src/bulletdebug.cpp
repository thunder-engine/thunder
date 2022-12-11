#include "bulletdebug.h"

#ifdef SHARED_DEFINE
#include <viewport/handles.h>

void BulletDebug::drawLine(const btVector3& from, const btVector3& to, const btVector3& color) {
    m_points.push_back(Vector3(from.x(), from.y(), from.z()));
    m_points.push_back(Vector3(to.x(), to.y(), to.z()));

    m_indices.push_back(m_indices.size());
    m_indices.push_back(m_indices.size());
}

void BulletDebug::clearLines() {
    m_points.clear();
    m_indices.clear();
}

void BulletDebug::flushLines() {
    Handles::drawLines(Matrix4(), Vector3Vector(m_points.begin(), m_points.end()), IndexVector(m_indices.begin(), m_indices.end()) );
}
#endif
