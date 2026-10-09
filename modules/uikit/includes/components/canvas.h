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

#include <widget.h>
#include <uidocument.h>
#include <stylesheet.h>

class CommandBuffer;
class RectTransform;
class RenderTarget;
class Texture;
class Mesh;
class MaterialInstance;

class UIKIT_EXPORT Canvas : public Widget {
    A_OBJECT(Canvas, Widget, Components/UI)

    A_PROPERTIES(
        A_PROPERTYEX(UiDocument *, document, Canvas::document, Canvas::setDocument, "editor=Asset"),
        A_PROPERTYEX(StyleSheet *, styleSheet, Canvas::styleSheet, Canvas::setStyleSheet, "editor=Asset")
    )
    A_METHODS(
        A_METHOD(void, Canvas::fromBuffer),
        A_SIGNAL(Canvas::documentLoaded)
    )
    A_NOENUMS()

public:
    Canvas();

    void markDirty();

    void update(const Vector2 &position) override;

    void draw(CommandBuffer *buffer);

    void drawRect(MaterialInstance *material, RectTransform *transform);

    void drawMesh(Mesh *mesh, MaterialInstance *material);

    void setSize(int width, int height);

    RectTransform *rectTransform();
    void setRectTransform(RectTransform *transform);

    void setClipRegion(const Vector4 &region);
    void disableClip();

    Texture *texture() const;

    void skipRenderResult(bool skip);

    UiDocument *document() const;
    void setDocument(UiDocument *document);

    StyleSheet *styleSheet() const;
    void setStyleSheet(StyleSheet *style);

    TString documentStyle() const;
    void fromBuffer(const TString &buffer);
    void documentLoaded();

private:
    void composeComponent() override;

    void resolveStyleSheet(Widget *widget);
    void cleanHierarchy(Widget *widget);

    static void materialUpdated(int state, void *ptr);

private:
    Vector2 m_lastPosition;

    TString m_documentStyle;

    RenderTarget *m_target;

    Texture *m_texture;

    CommandBuffer *m_buffer;

    MaterialInstance *m_finalMaterial;

    UiDocument *m_document;

    StyleSheet *m_styleSheet;

    bool m_dirty;

    bool m_lastPositionValid;

};

#endif // CANVAS_H
