#ifndef MESHGL_H
#define MESHGL_H

#include <list>

#include <resources/mesh.h>
#include <amath.h>

class CommandBufferGL;

class AMeshGL : public Mesh {
    A_OVERRIDE(AMeshGL, Mesh, Resources)
public:
    typedef vector<IndexVector> BufferVector;

public:
    AMeshGL                     ();

    ~AMeshGL                    ();

    void                        apply               ();

    void                        clear               ();

    void                        subscribe           (CommandBufferGL *buffer);
    void                        unsubscribe         (CommandBufferGL *buffer);

public:
    BufferVector                m_triangles;
    BufferVector                m_uv0;
    BufferVector                m_uv1;
    BufferVector                m_uv2;
    BufferVector                m_uv3;
    BufferVector                m_normals;
    BufferVector                m_tangents;
    BufferVector                m_vertices;
    BufferVector                m_colors;
    BufferVector                m_weights;
    BufferVector                m_indices;

    list<CommandBufferGL *>     m_Listeners;

};

#endif // MESHGL_H
