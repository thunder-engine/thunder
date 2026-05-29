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
        m_imageProgress(nullptr),
        m_frameProgress(nullptr),
        m_orientation(Horizontal),
        m_from(0.0f),
        m_to(1.0f),
        m_value(0.0f),
        m_dirtyProgress(true) {

    Material *spriteMaterial = Engine::loadResource<Material>(gDefaultSprite);
    if(spriteMaterial) {
        m_imageProgress = spriteMaterial->createInstance();
        m_imageProgress->setVector4(gColor, &m_backgroundColor);
    }

    Material *frameMaterial = Engine::loadResource<Material>(gDefaultFrame);
    if(frameMaterial) {
        m_frameProgress = frameMaterial->createInstance();

        Vector4 width(0.0f);
        m_frameProgress->setVector4(gBorderWidth, &width);
        m_frameProgress->setVector4(gBorderRadius, &m_borderRadius);
        m_frameProgress->setVector4(gBorderColor, &m_borderColor);
        m_frameProgress->setVector4(gBackgroundColor, &m_backgroundColor);
    }
}

ProgressBar::~ProgressBar() {
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
        m_frameProgress->setVector4(gBackgroundColor, &m_backgroundColor);
    }

    if(m_imageProgress) {
        m_imageProgress->setVector4(gColor, &m_backgroundColor);
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
            m_progressMesh = Engine::objectCreate<Mesh>();
            m_progressMesh->makeDynamic();

            Vector2 size(rect->size());
            m_backgroundImage->composeMesh(m_progressMesh, Sprite::Sliced, size);

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
