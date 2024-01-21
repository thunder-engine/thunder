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
