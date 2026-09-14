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
#ifndef MESHGL_H
#define MESHGL_H

#include <list>

#include <resources/mesh.h>
#include <amath.h>

class CommandBufferGL;
class MaterialGL;

struct VaoStruct {
    bool dirty;
    CommandBufferGL *buffer;
    uint32_t vao;
};

class MeshGL : public Mesh {
    A_OBJECT_OVERRIDE(MeshGL, Mesh, Resources)

    A_NOPROPERTIES()
    A_NOMETHODS()

public:
    typedef IndexVector BufferVector;

public:
    MeshGL();

    void bindVao(CommandBufferGL *buffer);

protected:
    void updateVao();
    void updateVbo();

public:
    uint32_t m_triangles;
    uint32_t m_vertices;

    std::list<VaoStruct *> m_vao;

};

#endif // MESHGL_H
