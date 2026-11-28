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

#ifndef SPRITERENDER
#define SPRITERENDER

#include <renderable.h>

#include <sprite.h>
#include <material.h>

class Texture;
class MaterialInstance;
class Mesh;

class ENGINE_EXPORT SpriteRender : public Renderable {
    A_OBJECT(SpriteRender, Renderable, Components/2D)

    A_PROPERTIES(
        A_PROPERTYEX(Material *, material, SpriteRender::material, SpriteRender::setMaterial, "editor=Asset"),
        A_PROPERTYEX(Sprite *, sprite, SpriteRender::sprite, SpriteRender::setSprite, "editor=Asset"),
        A_PROPERTYEX(Vector4, color, SpriteRender::color, SpriteRender::setColor, "editor=Color"),
        A_PROPERTY(Vector2, size, SpriteRender::size, SpriteRender::setSize),
        A_PROPERTYEX(DrawMode, drawMode, SpriteRender::drawMode, SpriteRender::setDrawMode, "enum=DrawMode"),
        A_PROPERTY(int, layer, SpriteRender::layer, SpriteRender::setLayer)
    )
    A_NOMETHODS()
    A_ENUMS(
        A_ENUM(DrawMode,
               A_VALUE(Simple),
               A_VALUE(Sliced),
               A_VALUE(Tiled))
    )

public:
    enum DrawMode {
        Simple = 0,
        Sliced,
        Tiled
    };

public:
    SpriteRender();
    ~SpriteRender();

    Sprite *sprite() const;
    void setSprite(Sprite *sprite);

    Texture *texture() const;
    void setTexture(Texture *texture);

    Vector4 color() const;
    void setColor(const Vector4 &color);

    Vector2 size() const;
    void setSize(const Vector2 &size);

    int drawMode() const;
    void setDrawMode(int mode);

    int layer() const;
    void setLayer(int layer);

    float blendShapeWeight(size_t index) const;
    void setBlendShapeWeight(size_t index, float weight);

    void setMaterial(Material *material) override;
    void setMesh(Mesh *mesh);

    MaterialInstance *materialInstance(int index) override;

protected:
    Mesh *meshToDraw() override;

    void setMaterialsList(const std::list<Material *> &materials) override;

    AABBox localBound() override;

    void composeComponent() override;

    Mesh *ensureMeshInstance();

    static void spriteUpdated(int state, void *ptr);

private:
    std::vector<float> m_blendShapeWeights;

    Vector4 m_color;

    Vector2 m_size;

    Sprite *m_sprite;

    Texture *m_texture;

    Mesh *m_mesh;

    Mesh *m_customMesh;
    Mesh *m_customMeshInstance;

    int m_drawMode;

    int m_priority;

    bool m_useCustom;

    bool m_dirtyMesh;
    bool m_dirtyMaterial;

};

#endif // SPRITERENDER

