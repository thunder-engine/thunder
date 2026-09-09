#include "utils/debugrender.h"

#include <gizmos.h>

#include <DetourDebugDraw.h>

void DebugRender::begin(duDebugDrawPrimitives prim, float size) {
    m_primitive = prim;
    m_vertices.clear();
    m_indices.clear();
}

void DebugRender::vertex(const float x, const float y, const float z, unsigned int color) {
    m_vertices.push_back(Vector3(x, y, z));
    m_indices.push_back(m_indices.size());
}

void DebugRender::vertex(const float *pos, unsigned int color) {
    vertex(pos[0], pos[1], pos[2], color);
}

void DebugRender::vertex(const float *pos, unsigned int color, const float *uv) {
    vertex(pos, color);
}

void DebugRender::vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v) {
    vertex(x, y, z, color);
}

void DebugRender::end() {
    if(m_vertices.empty()) {
        return;
    }

    switch(m_primitive) {
        case DU_DRAW_LINES: Gizmos::drawLines(m_vertices, m_indices, Vector4(0.24f, 0.54f, 0.77f, 1.0f)); break;
        case DU_DRAW_TRIS: Gizmos::drawSolidTriangles(m_vertices, m_indices, Vector4(0.24f, 0.54f, 0.77f, 0.5f)); break;
        default: break;
    }
}

void DebugRender::draw(dtNavMesh *mesh) {
    duDebugDrawNavMesh(this, *mesh, DU_DRAWNAVMESH_COLOR_TILES);
}
