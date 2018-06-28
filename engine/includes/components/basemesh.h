#ifndef BASEMESH_H
#define BASEMESH_H

#include "mesh.h"
#include "material.h"

#include "component.h"

#include <array>

class NEXT_LIBRARY_EXPORT BaseMesh : public Component {
    A_REGISTER(BaseMesh, Component, General);

public:
    BaseMesh                    ();

    Mesh                       *mesh                    () const;

    virtual void                setMesh                 (Mesh *mesh);

    MaterialArray               materials               () const;

    virtual void                setMaterials            (const MaterialArray &material);

    Material                   *material                (uint32_t index = 0) const;

    virtual void                setMaterial             (Material *material, uint32_t index = 0);

    uint32_t                    materialCount           () const;

    void                        loadUserData            (const VariantMap &data);

    VariantMap                  saveUserData            () const;

protected:
    Mesh                       *m_pMesh;

    MaterialArray               m_Materials;

};

#endif // BASEMESH_H
