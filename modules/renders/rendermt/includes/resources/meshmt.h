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
#ifndef MESHMT_H
#define MESHMT_H

#include <list>

#include <resources/mesh.h>
#include <amath.h>

#include "wrappermt.h"

class CommandBufferMt;

class MeshMt : public Mesh {
    A_OBJECT_OVERRIDE(MeshMt, Mesh, Resources)

    A_NOPROPERTIES()
    A_NOMETHODS()

public:
    MeshMt();

    MTL::Buffer *indexBuffer();

    void bind(MTL::RenderCommandEncoder *encoder, int uniformOffset);

protected:
    void update();

protected:
    MTL::Buffer *m_vertexBuffer;

    MTL::Buffer *m_indexBuffer;

    uint32_t m_vertexSize;

    uint32_t m_uvSize;

    uint32_t m_colorSize;

    uint32_t m_normalsSize;

    uint32_t m_tangentsSize;

    uint32_t m_weightsSize;

    uint32_t m_bonesSize;

};

#endif // MESHMT_H
