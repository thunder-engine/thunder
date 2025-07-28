#ifndef SPRITEELEMENT_H
#define SPRITEELEMENT_H

#include <amath.h>

#include "../converter/textureconverter.h"

class SpriteController;

class SpriteElement : public Object {
    A_OBJECT(SpriteElement, Object, Editor)

    A_PROPERTIES(
        A_PROPERTY(TString, name, SpriteElement::name, SpriteElement::setName),
        A_PROPERTY(Vector2, position, SpriteElement::position, SpriteElement::setPosition),
        A_PROPERTY(Vector2, size, SpriteElement::size, SpriteElement::setSize),
        A_PROPERTY(Vector4, borderTRBL, SpriteElement::border, SpriteElement::setBorder),
        A_PROPERTY(Vector2, pivot, SpriteElement::pivot, SpriteElement::setPivot)
    )

public:
    SpriteElement();

    void setController(SpriteController *controller);
    void setSettings(TextureImportSettings *settings);

    TString key() const { return m_key; }
    void setKey(const TString &key);

    TString name() const;
    void setName(const TString &name);

    Vector2 position() const;
    void setPosition(const Vector2 &position);

    Vector2 size() const;
    void setSize(const Vector2 &position);

    Vector2 pivot() const;
    void setPivot(const Vector2 &pivot);

    Vector4 border() const;
    void setBorder(const Vector4 &border);

private:
    void updateController(const TextureImportSettings::Element &element);

private:
    TString m_key;

    SpriteController *m_controller;

    TextureImportSettings *m_settings;

};

#endif // SPRITEELEMENT_H
