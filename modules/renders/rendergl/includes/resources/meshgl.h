#ifndef MESHGL_H
#define MESHGL_H

#include <list>

#include <resources/mesh.h>
#include <amath.h>

class CommandBufferGL;

struct VaoStruct {
    bool dirty;
    CommandBufferGL *buffer;
    uint32_t vao;
};

class MeshGL : public Mesh {
    A_OVERRIDE(MeshGL, Mesh, Resources)

    A_NOPROPERTIES()
    A_NOMETHODS()

public:
    typedef IndexVector BufferVector;

public:
    MeshGL();

    void bindVao(CommandBufferGL *buffer);

    uint32_t instance() const;

protected:
    void updateVao();
    void updateVbo(CommandBufferGL *buffer);

    void destroyVao(CommandBufferGL *buffer);
    void destroyVbo();

public:
    uint32_t m_triangles;
    uint32_t m_uv0;
    uint32_t m_uv1;
    uint32_t m_normals;
    uint32_t m_tangents;
    uint32_t m_vertices;
    uint32_t m_colors;
    uint32_t m_weights;
    uint32_t m_bones;

    uint32_t m_instanceBuffer;

    list<VaoStruct *> m_vao;
};

#endif // MESHGL_H
