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
#ifndef CANVAS_H
#define CANVAS_H

#include <component.h>
#include <uikit.h>

class CommandBuffer;
class RectTransform;
class RenderTarget;
class Texture;
class Mesh;
class MaterialInstance;

class UIKIT_EXPORT Canvas : public Component {
    A_OBJECT(Canvas, Component, Components/UI)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    Canvas();

    void markDirty();

    void update(const Vector2 &position);

    void draw(CommandBuffer *buffer);

    void drawRect(MaterialInstance *material, RectTransform *transform);

    void drawMesh(Mesh *mesh, MaterialInstance *material);

    void setSize(int width, int height);

    RectTransform *rectTransform();
    void setRectTransform(RectTransform *transform);

    void setClipRegion(const Vector4 &region);
    void disableClip();

private:
    void composeComponent() override;

private:
    RenderTarget *m_target;

    Texture *m_texture;

    RectTransform *m_transform;

    CommandBuffer *m_buffer;

    MaterialInstance *m_finalMaterial;

    bool m_dirty;

    bool m_lastPositionValid;
    Vector2 m_lastPosition;

};

#endif // CANVAS_H
