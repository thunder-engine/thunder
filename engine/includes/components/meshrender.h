#ifndef MESHRENDER_H
#define MESHRENDER_H

#include "mesh.h"
#include "material.h"

#include "renderable.h"

#include <array>

class NEXT_LIBRARY_EXPORT MeshRender : public Renderable {
    A_REGISTER(MeshRender, Renderable, General);

    A_PROPERTIES (
        A_PROPERTY(Mesh *, Mesh, MeshRender::mesh, MeshRender::setMesh),
        A_PROPERTY(MateralArray, Materials, MeshRender::materials, MeshRender::setMaterials)
    )

    A_METHODS(
        A_METHOD(Material *, MeshRender::material),
        A_METHOD(void, MeshRender::setMaterial)
    )

public:
    MeshRender                  ();

    void                        draw                    (ICommandBuffer &buffer, int8_t layer);

    Mesh                       *mesh                    () const;

    virtual void                setMesh                 (Mesh *mesh);

    MaterialArray               materials               () const;

    virtual void                setMaterials            (const MaterialArray &material);

    Material                   *material                (int index = 0) const;

    virtual void                setMaterial             (Material *material, int index = 0);

    uint32_t                    materialCount           () const;

    void                        loadUserData            (const VariantMap &data);

    VariantMap                  saveUserData            () const;

protected:
    Mesh                       *m_pMesh;

    MaterialArray               m_Materials;

};

#endif // MESHRENDER_H
