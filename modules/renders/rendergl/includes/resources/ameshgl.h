#ifndef MESHGL_H
#define MESHGL_H

#include <list>

#include <resources/mesh.h>
#include <amath.h>

class CommandBufferGL;

class AMeshGL : public Mesh {
    A_OVERRIDE(AMeshGL, Mesh, Resources)
public:
    typedef IndexVector         BufferVector;

public:
    AMeshGL                     ();

    void                        bindVao             (CommandBufferGL *buffer, uint32_t lod);

    uint32_t                    instance            () const;

protected:
    void                        updateVao           (uint32_t lod);
    void                        updateVbo           ();

    void                        destroyVao          ();
    void                        destroyVbo          ();

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
    IndexVector m_indices;

    uint32_t m_InstanceBuffer;

    typedef map<CommandBufferGL *, uint32_t>  VaoMap;

    typedef vector<VaoMap> VaoVector;

    VaoVector m_Vao;
};

#endif // MESHGL_H
