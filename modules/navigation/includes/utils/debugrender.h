#ifndef DEBUGRENDER_H
#define DEBUGRENDER_H

#include <DebugDraw.h>
#include <amath.h>

class dtNavMesh;

struct DebugRender : public duDebugDraw {
public:
    void draw(dtNavMesh *mesh);

protected:
    void depthMask(bool state) override {}
    void texture(bool state) override {}

    void begin(duDebugDrawPrimitives prim, float size = 1.0f) override;
    void vertex(const float *pos, unsigned int color) override;
    void vertex(const float x, const float y, const float z, unsigned int color) override;
    void vertex(const float *pos, unsigned int color, const float *uv) override;
    void vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v) override;
    void end() override;

protected:
    duDebugDrawPrimitives m_primitive;

    Vector3Vector m_vertices;
    IndexVector m_indices;
};

#endif // DEBUGRENDER_H
