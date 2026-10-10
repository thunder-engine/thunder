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
#include "components/progressbar.h"

#include "components/recttransform.h"
#include "components/canvas.h"

#include <components/actor.h>

#include <resources/material.h>

namespace {
    const char *gBackgroundColor("backgroundColor");
    const char *gBorderWidth("borderWidth");
    const char *gBorderRadius("borderRadius");
    const char *gBorderColor("borderColor");

    const char *gOverride("mainTexture");
    const char *gColor("mainColor");

    const char *gDefaultSprite(".embedded/DefaultUI.shader");
    const char *gDefaultFrame(".embedded/Frame.shader");
}

/*!
    \class ProgressBar
    \brief The ProgressBar class represents a graphical user interface element that displays progress visually.
    \inmodule Gui

    The ProgressBar class is designed to provide a graphical representation of progress with customizable appearance and range.
    It supports features such as setting the minimum and maximum values, adjusting the progress value, and specifying visual elements for background and progress indicator.
*/

ProgressBar::ProgressBar() :
        m_progressColor(1.0f, 1.0f, 1.0f, 1.0f),
        m_progressImage(nullptr),
        m_progressMesh(nullptr),
        m_spriteMaterial(nullptr),
        m_defaultFrameMaterial(nullptr),
        m_imageProgress(nullptr),
        m_frameProgress(nullptr),
        m_orientation(Horizontal),
        m_from(0.0f),
        m_to(1.0f),
        m_value(0.0f),
        m_dirtyProgress(true) {

    m_spriteMaterial = Engine::loadResource<Material>(gDefaultSprite);
    if(m_spriteMaterial) {
        m_imageProgress = m_spriteMaterial->createInstance();
        m_imageProgress->setVector4(gColor, &m_progressColor);
        m_spriteMaterial->subscribe(&ProgressBar::materialUpdated, this);
    }

    m_defaultFrameMaterial = Engine::loadResource<Material>(gDefaultFrame);
    if(m_defaultFrameMaterial) {
        m_frameProgress = m_defaultFrameMaterial->createInstance();

        Vector4 width(0.0f);
        m_frameProgress->setVector4(gBorderWidth, &width);
        m_frameProgress->setVector4(gBorderRadius, &m_borderRadius);
        m_frameProgress->setVector4(gBorderColor, &m_borderColor);
        m_frameProgress->setVector4(gBackgroundColor, &m_progressColor);
        m_defaultFrameMaterial->subscribe(&ProgressBar::materialUpdated, this);
    }
}

ProgressBar::~ProgressBar() {
    if(m_spriteMaterial) {
        m_spriteMaterial->unsubscribe(this);
    }
    if(m_defaultFrameMaterial) {
        m_defaultFrameMaterial->unsubscribe(this);
    }

    delete m_progressImage;
    delete m_progressMesh;

    delete m_imageProgress;
    m_imageProgress = nullptr;

    delete m_frameProgress;
    m_frameProgress = nullptr;
}
/*!
    Returns the orientation of the progress bar.
*/
int ProgressBar::orientation() const {
    return m_orientation;
}
/*!
    Sets the \a orientation of the progress bar.
*/
void ProgressBar::setOrientation(int orientation) {
    if(m_orientation != orientation) {
        m_orientation = orientation;

        repaint();
    }
}
/*!
    Returns the minimum value of the progress range.
*/
float ProgressBar::from() const {
    return m_from;
}
/*!
    Sets the minimum \a value of the progress range.
*/
void ProgressBar::setFrom(float value) {
    if(m_from != value) {
        m_from = value;

        repaint();
    }
}
/*!
    Returns the maximum value of the progress range.
*/
float ProgressBar::to() const {
    return m_to;
}
/*!
    Sets the maximum \a value of the progress range.
*/
void ProgressBar::setTo(float value) {
    if(m_to != value) {
        m_to = value;

        repaint();
    }
}
/*!
    Returns the current progress value.
*/
float ProgressBar::value() const {
    return m_value;
}
/*!
    Sets the current progress \a value.
*/
void ProgressBar::setValue(float value) {
    if(m_value != value) {
        m_value = value;

        repaint();
    }
}
/*!
    Returns the color of the progress indicator.
*/
Vector4 ProgressBar::progressColor() const {
    return m_progressColor;
}
/*!
    Sets the \a color of the progress indicator.
*/
void ProgressBar::setProgressColor(const Vector4 color) {
    m_progressColor = color;

    if(m_frameProgress) {
        m_frameProgress->setVector4(gBackgroundColor, &m_progressColor);
    }

    if(m_imageProgress) {
        m_imageProgress->setVector4(gColor, &m_progressColor);
    }

    repaint();
}
/*!
    Returns progress image.
*/
Sprite *ProgressBar::progressImage() const {
    return m_progressImage;
}
/*!
    Sets progress \a image.
*/
void ProgressBar::setProgressImage(Sprite *image) {
    if(m_progressImage != image) {
        m_progressImage = image;

        m_dirtyProgress = true;
        repaint();
    }
}
/*!
    \internal
    Internal method called to draw progress bar.
*/
void ProgressBar::draw() {
    Frame::draw();

    RectTransform *rect = rectTransform();
    if(m_dirtyProgress) {
        if(m_progressImage) {
            delete m_progressMesh;
            m_progressMesh = Engine::objectCreate<Mesh>();
            m_progressMesh->makeDynamic();

            Vector2 size(rect->size());
            m_progressImage->composeMesh(m_progressMesh, Sprite::Sliced, size);

            m_imageProgress->setTexture(gOverride, m_progressImage->texture());
        }
        m_dirtyProgress = false;
    }

    Canvas *canvas = Frame::canvas();

    Vector4 clip = rect->clipRegion();
    if(m_orientation == Horizontal) {
        clip.z = clip.z / (m_to - m_from) * m_value;
    } else {
        clip.w = clip.w / (m_to - m_from) * m_value;
    }

    canvas->setClipRegion(clip);
    if(m_progressImage) {
        Matrix4 mat(rect->worldTransform());

        const Vector3Vector &verts(m_progressMesh->vertices());
        Vector2 scl(rect->worldScale());
        mat[12] -= verts[0].x * scl.x;
        mat[13] -= verts[0].y * scl.y;

        uint32_t hash = rect->hash();
        Mathf::hashCombine(hash, mat[12]);
        Mathf::hashCombine(hash, mat[13]);

        m_imageProgress->setTransform(mat, 0, hash);

        canvas->drawMesh(m_progressMesh, m_imageProgress);
    } else {
        canvas->drawRect(m_frameProgress, rect);
    }
    canvas->disableClip();
}
/*!
    \internal
    Composes the components of the progress bar and sets initial properties.
*/
void ProgressBar::composeComponent() {
    Widget::composeComponent();

    setValue(0.5f);

    rectTransform()->setSize(Vector2(100.0f, 20.0f));
}

void ProgressBar::materialUpdated(int state, void *ptr) {
    if(state > Material::Ready) {
        return;
    }

    ProgressBar *progressBar = static_cast<ProgressBar *>(ptr);
    if(!progressBar) {
        return;
    }

    if(progressBar->m_frameProgress) {
        RectTransform *rect = progressBar->rectTransform();
        const float height = MAX(rect->size().y, 1.0f);
        Vector4 borderRadius(progressBar->m_borderRadius / height);
        Vector4 zeroWidth(0.0f);
        progressBar->m_frameProgress->setVector4(gBorderWidth, &zeroWidth);
        progressBar->m_frameProgress->setVector4(gBorderRadius, &borderRadius);
        progressBar->m_frameProgress->setVector4(gBorderColor, &progressBar->m_borderColor);
        progressBar->m_frameProgress->setVector4(gBackgroundColor, &progressBar->m_progressColor);
    }

    if(progressBar->m_imageProgress) {
        progressBar->m_imageProgress->setVector4(gColor, &progressBar->m_progressColor);
        if(progressBar->m_progressImage) {
            progressBar->m_imageProgress->setTexture(gOverride, progressBar->m_progressImage->texture());
        }
    }

    progressBar->m_dirtyProgress = true;
    progressBar->repaint();
}
