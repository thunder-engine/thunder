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
