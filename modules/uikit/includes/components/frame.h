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
#ifndef FRAME_H
#define FRAME_H

#include "widget.h"

#include <sprite.h>

class Mesh;
class MaterialInstance;

class UIKIT_EXPORT Frame : public Widget {
    A_OBJECT(Frame, Widget, Components/UI)

    A_PROPERTIES(
        A_PROPERTY(Vector4, corners, Frame::corners, Frame::setCorners),
        A_PROPERTYEX(Vector4, backgroundColor, Frame::backgroundColor, Frame::setBackgroundColor, "editor=Color, css=background-color"),
        A_PROPERTYEX(Vector4, borderColor, Frame::borderColor, Frame::setBorderColor, "editor=Color, css=border-color"),
        A_PROPERTYEX(Sprite *, backgroundImage, Frame::backgroundImage, Frame::setBackgroundImage, "editor=Asset")
    )
    A_NOMETHODS()
    A_NOENUMS()

public:
    Frame();
    ~Frame();

    Vector4 corners() const;
    void setCorners(const Vector4 &corners);

    Vector4 backgroundColor() const;
    void setBackgroundColor(const Vector4 &color);

    Vector4 borderColor() const;
    void setBorderColor(const Vector4 &color);

    Sprite *backgroundImage() const;
    void setBackgroundImage(Sprite *image);

protected:
    void boundChanged(const Vector2 &size) override;

    void draw() override;

    void applyStyle() override;

protected:
    Vector4 m_borderRadius;
    Vector4 m_backgroundColor;
    Vector4 m_borderColor;

    Sprite *m_backgroundImage;

    Mesh *m_backgroundMesh;

    MaterialInstance *m_imageMaterial;
    MaterialInstance *m_frameMaterial;

    bool m_dirtyBackground;

};

#endif // FRAME_H
