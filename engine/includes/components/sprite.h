#ifndef SPRITE
#define SPRITE

#include "component.h"

#include "resources/material.h"

class NEXT_LIBRARY_EXPORT Sprite : public Component {
    A_REGISTER(Sprite, Component, Components);

    A_PROPERTIES(
        A_PROPERTY(Material*, Material, Sprite::material, Sprite::setMaterial),
        A_PROPERTY(Texture*, Texture, Sprite::texture, Sprite::setTexture)
    );
    A_NOMETHODS();

public:
    Sprite                      ();

    void                        update              ();

    Vector2                     center              () const;

    void                        setCenter           (const Vector2 &value);

    Material                   *material            () const;

    virtual void                setMaterial         (Material *material);

    Texture                    *texture             () const;

    virtual void                setTexture          (Texture *texture);

    void                        loadUserData        (const VariantMap &data);

    VariantMap                  saveUserData        () const;

protected:
    Vector2                     m_Center;

    MaterialInstance           *m_Material;

    Texture                    *m_Texture;

};

#endif // SPRITE

