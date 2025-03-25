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
    void updateVbo(CommandBufferGL *buffer);

public:
    uint32_t m_triangles;
    uint32_t m_vertices;

    std::list<VaoStruct *> m_vao;

};

#endif // MESHGL_H
