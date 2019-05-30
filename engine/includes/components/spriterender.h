#ifndef SPRITERENDER
#define SPRITERENDER

#include "meshrender.h"

class NEXT_LIBRARY_EXPORT SpriteRender : public Renderable {
    A_REGISTER(SpriteRender, Renderable, Components)

    A_PROPERTIES(
        A_PROPERTY(Material*, Material, SpriteRender::material, SpriteRender::setMaterial),
        A_PROPERTY(Texture*, Texture, SpriteRender::texture, SpriteRender::setTexture)
    )
    A_NOMETHODS()

public:
    SpriteRender                ();

    void                        draw                (ICommandBuffer &buffer, uint32_t layer);

    Material                   *material            () const;
    void                        setMaterial         (Material *material);

    Texture                    *texture             () const;
    virtual void                setTexture          (Texture *texture);

    Mesh                       *mesh                () const;

    void                        loadUserData        (const VariantMap &data);
    VariantMap                  saveUserData        () const;

protected:
    Texture                    *m_Texture;

    MaterialInstance           *m_pMaterial;

    Mesh                       *m_pMesh;

};

#endif // SPRITERENDER

