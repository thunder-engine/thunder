#ifndef SPRITEMESH
#define SPRITEMESH

#include "component.h"

#include "material.h"

class Mesh;

class NEXT_LIBRARY_EXPORT SpriteMesh : public Component {
    A_REGISTER(SpriteMesh, Component, Components);

    A_PROPERTIES(
        A_PROPERTY(Material*, Material, SpriteMesh::material, SpriteMesh::setMaterial),
        A_PROPERTY(Texture*, Texture, SpriteMesh::texture, SpriteMesh::setTexture)
    );
    A_NOMETHODS();

public:
    SpriteMesh                   ();

    void                        draw                (ICommandBuffer &buffer, int8_t layer);

    Vector2                     center              () const;

    void                        setCenter           (const Vector2 &value);

    Material                   *material            () const;

    virtual void                setMaterial         (Material *material);

    Texture                    *texture             () const;

    virtual void                setTexture          (Texture *texture);

    void                        loadUserData        (const VariantMap &data);

    VariantMap                  saveUserData        () const;

    Mesh                       *mesh                () const;

protected:
    Vector2                     m_Center;

    MaterialInstance           *m_Material;

    Texture                    *m_Texture;

    Mesh                       *m_pMesh;

};

#endif // SPRITEMESH

