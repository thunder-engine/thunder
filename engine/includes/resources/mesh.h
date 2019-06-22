#ifndef MESH_H
#define MESH_H

#include <memory>
#include <string>
#include <vector>
#include <list>
#include <map>

#include <amath.h>

#include "engine.h"

class Material;
class MeshPrivate;

class NEXT_LIBRARY_EXPORT Mesh : public Object {
    A_REGISTER(Mesh, Object, Resources)

public:
    enum Attributes {
        ATTRIBUTE_COLOR         = (1<<0),
        ATTRIBUTE_UV0           = (1<<1),
        ATTRIBUTE_UV1           = (1<<2),
        ATTRIBUTE_NORMALS       = (1<<3),
        ATTRIBUTE_TANGENTS      = (1<<4),
        ATTRIBUTE_ANIMATED      = (1<<5),
    };

    enum Modes {
        MODE_TRIANGLES          = 0,
        MODE_LINES,
        MODE_TRIANGLE_STRIP,
        MODE_LINE_STRIP
    };

    typedef vector<uint32_t>    IndexVector;

    struct Lod {
        IndexVector             indices;

        Vector2Vector           uv0;

        Vector2Vector           uv1;

        Vector3Vector           normals;

        Vector3Vector           tangents;

        Vector3Vector           vertices;

        Vector4Vector           colors;

        Vector4Vector           weights;

        Vector4Vector           bones;

        Material               *material;
    };
    typedef deque<Lod>          LodQueue;

public:
    Mesh                        ();
    virtual ~Mesh               ();

    virtual void                apply               ();
    virtual void                clear               ();

    bool                        isDynamic           () const;
    void                        makeDynamic         ();

    Material                   *material            (uint32_t lod) const;

    Vector3Vector               vertices            (uint32_t lod) const;

    IndexVector                 indices             (uint32_t lod) const;

    uint32_t                    lodsCount           () const;

    uint32_t                    vertexCount         (uint32_t lod) const;

    uint32_t                    indexCount          (uint32_t lod) const;

    AABBox bound () const;
    void setBound (const AABBox &box);

    Modes mode () const;
    void setMode (Mesh::Modes mode);

    uint8_t flags () const;
    void setFlags (uint8_t flags);

    void addLod (const Lod &lod);
    void setLod (uint32_t index, const Lod &lod);

protected:
    Lod *getLod(uint32_t lod) const;

private:
    void loadUserData (const VariantMap &data) override;

private:
    MeshPrivate *p_ptr;

};

#endif // MESH_H
