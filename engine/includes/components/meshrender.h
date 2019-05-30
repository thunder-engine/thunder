#ifndef MESHRENDER_H
#define MESHRENDER_H

#include "mesh.h"
#include "material.h"

#include "renderable.h"

#include <array>

class NEXT_LIBRARY_EXPORT MeshRender : public Renderable {
    A_REGISTER(MeshRender, Renderable, Components);

    A_PROPERTIES (
        A_PROPERTY(Mesh *, Mesh, MeshRender::mesh, MeshRender::setMesh),
        A_PROPERTY(Material *, Material, MeshRender::material, MeshRender::setMaterial)
    )
    A_NOMETHODS()

public:
    MeshRender                  ();

    void                        draw                    (ICommandBuffer &buffer, uint32_t layer);

    AABBox                      bound                   () const override;

    Mesh                       *mesh                    () const;

    virtual void                setMesh                 (Mesh *mesh);

    Material                   *material                () const;

    virtual void                setMaterial             (Material *material);

    void                        loadUserData            (const VariantMap &data);

    VariantMap                  saveUserData            () const;

protected:
    Mesh                       *m_pMesh;

    MaterialInstance           *m_pMaterial;

};

#endif // MESHRENDER_H
