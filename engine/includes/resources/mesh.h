#ifndef MESH_H
#define MESH_H

#include <memory>
#include <string>
#include <vector>
#include <list>
#include <map>

#include <amath.h>

#include "engine.h"
#include "material.h"

class NEXT_LIBRARY_EXPORT Mesh : public AObject {
    A_REGISTER(Mesh, AObject, Resources)
public:
    /*! \struct Vertex */
    struct Vertex {
        Vector4               xyz;

        Vector3               t;
        Vector3               n;

        Vector2               uv0;
        Vector2               uv1;

        Vector4               index;
        Vector4               weight;
    };

    typedef vector<Vertex>      VertexVector;
    typedef vector<uint32_t>    IndexVector;

    struct Lod {
        /// Vertices array
        VertexVector            vertices;
        /// Indices array
        IndexVector             indices;

        Material               *material;
    };
    typedef deque<Lod>          LodQueue;

    /*! \struct Surface */
    struct Surface {
        LodQueue                lods;
        /// Special tag indicating that a given surface in calculation of collision
        bool                    collision;

        OBBox                   obb;
    };
    typedef deque<Surface>      SurfaceQueue;

public:
    SurfaceQueue                m_Surfaces;

public:
    Mesh                        ();
    virtual ~Mesh               ();

    void                        loadUserData        (const AVariantMap &data);
    AVariantMap                 saveUserData        () const;

    bool                        isAnimated          () const;
    void                        setAnimated         (bool animated);

protected:
    bool                        m_Animated;

};

#endif // MESH_H
