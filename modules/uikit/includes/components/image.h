/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
#ifndef IMAGE_H
#define IMAGE_H

#include "widget.h"

#include <sprite.h>
#include <material.h>

class Mesh;
class Texture;
class MaterialInstance;

class UIKIT_EXPORT Image : public Widget {
    A_OBJECT(Image, Widget, Components/UI)

    A_PROPERTIES(
        A_PROPERTYEX(Material *, material, Image::material, Image::setMaterial, "editor=Asset"),
        A_PROPERTYEX(Sprite *, sprite, Image::sprite, Image::setSprite, "editor=Asset"),
        A_PROPERTYEX(Mode, drawMode, Image::drawMode, Image::setDrawMode, "enum=Mode"),
        A_PROPERTYEX(Vector4, color, Image::color, Image::setColor, "editor=Color")
    )
    A_NOMETHODS()
    A_ENUMS(
        A_ENUM(Mode,
            A_VALUE(Simple),
            A_VALUE(Sliced),
            A_VALUE(Tiled)
        )
    )

public:
    enum Mode {
        Simple = 0,
        Sliced,
        Tiled
    };

public:
    Image();
    ~Image();

    Material *material() const;
    virtual void setMaterial(Material *material);

    Sprite *sprite() const;
    void setSprite(Sprite *sprite);

    void setTexture(Texture *texture);

    Vector4 color() const;
    void setColor(const Vector4 &color);

    int drawMode() const;
    void setDrawMode(int mode);

protected:
    void draw() override;

    void boundChanged(const Vector2 &size) override;

    void makeDefaultMesh();

    static void spriteUpdated(int state, void *ptr);

protected:
    Vector4 m_color;

    Vector2 m_size;

    Mesh *m_mesh;

    Sprite *m_sprite;

    Texture *m_texture;

    MaterialInstance *m_material;

    int m_drawMode;

    bool m_dirtyMesh;

    bool m_dirtyMaterial;

};

#endif // IMAGE_H
