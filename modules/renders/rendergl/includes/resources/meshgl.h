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

    void bindVao(CommandBufferGL *buffer, uint32_t lod);

    uint32_t instance() const;

protected:
    void switchState(ResourceState state) override;

    void updateVao(uint32_t lod);
    void updateVbo(CommandBufferGL *buffer);

    void destroyVao(CommandBufferGL *buffer);
    void destroyVbo();

public:
    IndexVector m_triangles;
    IndexVector m_uv0;
    IndexVector m_uv1;
    IndexVector m_uv2;
    IndexVector m_uv3;
    IndexVector m_normals;
    IndexVector m_tangents;
    IndexVector m_vertices;
    IndexVector m_colors;
    IndexVector m_weights;
    IndexVector m_bones;

    uint32_t m_InstanceBuffer;

    typedef vector<list<VaoStruct *>> VaoVector;

    VaoVector m_Vao;
};

#endif // MESHGL_H
