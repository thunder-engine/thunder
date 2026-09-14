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
