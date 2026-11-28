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

#ifndef SPRITE_H
#define SPRITE_H

#include "texture.h"
#include "mesh.h"

class ENGINE_EXPORT Sprite : public Resource {
    A_OBJECT(Sprite, Resource, Resources)

    A_PROPERTIES(
        A_PROPERTY(TString, name, Sprite::name, Sprite::setName),
        A_PROPERTY(Vector4, bounds, Sprite::bounds, Sprite::setBounds),
        A_PROPERTY(Vector4, border, Sprite::border, Sprite::setBorder),
        A_PROPERTY(Vector2, pivot, Sprite::pivot, Sprite::setPivot),
        A_PROPERTYEX(int, mode, Sprite::mode, Sprite::setMode, "enum=SpriteMode"),
        A_PROPERTY(float, pixelsPerUnit, Sprite::pixelsPerUnit, Sprite::setPixelsPerUnit)
    )
    A_ENUMS(
        A_ENUM(SpriteMode,
            A_VALUE(Simple),
            A_VALUE(Sliced),
            A_VALUE(Tiled),
            A_VALUE(Complex))
    )

public:
    enum SpriteMode {
        Simple = 0,
        Sliced,
        Tiled,
        Complex
    };

public:
    Sprite();
    ~Sprite();

    Mesh *mesh();

    float pixelsPerUnit() const;
    void setPixelsPerUnit(float pixels);

    Vector4 bounds() const;
    void setBounds(const Vector4 &bounds);

    Vector4 border() const;
    void setBorder(const Vector4 &border);

    Vector2 pivot() const;
    void setPivot(const Vector2 &pivot);

    Texture *texture() const;
    void setTexture(Texture *texture);

    int mode() const;
    void setMode(int mode);

    Mesh *composeMesh(Mesh *mesh, SpriteMode mode, Vector2 &size);

private:
    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

    void recalcMesh() const;

    static void textureUpdated(int state, void *ptr);

private:
    Vector4 m_bounds;

    Vector4 m_border;

    Vector2 m_pivot;

    Texture *m_texture;

    Mesh *m_mesh;

    float m_pixelsPerUnit;

    int m_mode;

    bool m_dirtyMesh;

};

#endif // SPRITE_H
