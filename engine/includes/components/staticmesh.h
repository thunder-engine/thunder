#ifndef STATICMESH_H
#define STATICMESH_H

#include "resources/mesh.h"
#include "resources/material.h"

#include "component.h"

#include <array>

class NEXT_LIBRARY_EXPORT StaticMesh : public Component {
    A_REGISTER(StaticMesh, Component, Components)

    A_PROPERTIES (
        A_PROPERTY(Mesh*, Mesh, mesh, setMesh),
        A_PROPERTY(MateralArray, Materials, materials, setMaterials)
    )
    A_NOMETHODS()

public:
    StaticMesh                  ();

    void                        update                  ();

    Mesh                       *mesh                    () const;

    virtual void                setMesh                 (Mesh *mesh);

    MaterialArray               materials               () const;

    virtual void                setMaterials            (const MaterialArray &mat);

    Material                   *material                (uint32_t index) const;

    virtual void                setMaterial             (uint32_t index, Material *mat);

    uint32_t                    materialCount           () const;

    void                        loadUserData            (const AVariantMap &data);

    AVariantMap       saveUserData            () const;

protected:
    Mesh                       *m_pMesh;

    MaterialArray               m_Materials;

};

#endif // STATICMESH_H
