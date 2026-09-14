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
