#include "components/frame.h"

#include "components/canvas.h"
#include "components/recttransform.h"

#include <components/actor.h>
#include <components/spriterender.h>

#include <resources/sprite.h>
#include <resources/mesh.h>
#include <resources/material.h>
#include <resources/stylesheet.h>

#include <commandbuffer.h>
#include <gizmos.h>

namespace {
    const char *gBackgroundColor("backgroundColor");
    const char *gBorderWidth("borderWidth");
    const char *gBorderRadius("borderRadius");
    const char *gBorderColor("borderColor");

    const char *gOverride("mainTexture");
    const char *gColor("mainColor");

    const char *gCssBackgroundColor("background-color");
    const char *gCssBorderColor("border-color");
    const char *gCssBorderRadius("border-radius");

    const char *gDefaultSprite(".embedded/DefaultUI.shader");
    const char *gDefaultFrame(".embedded/Frame.shader");
}

/*!
    \class Frame
    \brief The Frame class represents a graphical frame or border with customizable corners, border width, and border color.
    \inmodule Gui

    The Frame class represents a graphical frame or border used in user interfaces.
    It is designed to visually group or contain other UI elements, providing a clear separation or visual boundary.
    The frame can have customizable corners, border width, and border color, making it a versatile element for organizing and structuring content within an application.
*/
Frame::Frame() :
        Widget(),
        m_borderRadius(0.0f),
        m_backgroundColor(1.0f, 1.0f, 1.0f, 0.5f),
        m_borderColor(0.8f),
        m_backgroundImage(nullptr),
        m_backgroundMesh(nullptr),
        m_imageMaterial(nullptr),
        m_frameMaterial(nullptr),
        m_dirtyBackground(true) {

    Material *spriteMaterial = Engine::loadResource<Material>(gDefaultSprite);
    if(spriteMaterial) {
        m_imageMaterial = spriteMaterial->createInstance();
        m_imageMaterial->setVector4(gColor, &m_backgroundColor);
    }

    Material *frameMaterial = Engine::loadResource<Material>(gDefaultFrame);
    if(frameMaterial) {
        m_frameMaterial = frameMaterial->createInstance();

        Vector4 width(0.0f);
        m_frameMaterial->setVector4(gBorderWidth, &width);
        m_frameMaterial->setVector4(gBorderRadius, &m_borderRadius);
        m_frameMaterial->setVector4(gBorderColor, &m_borderColor);
        m_frameMaterial->setVector4(gBackgroundColor, &m_backgroundColor);
    }
}

Frame::~Frame() {
    delete m_backgroundImage;
    delete m_backgroundMesh;

    delete m_imageMaterial;
    m_imageMaterial = nullptr;

    delete m_frameMaterial;
    m_frameMaterial = nullptr;
}
/*!
    \internal
*/
void Frame::draw() {
    RectTransform *rect = rectTransform();
    if(m_dirtyBackground) {
        if(m_backgroundImage) {
            m_backgroundMesh = Engine::objectCreate<Mesh>();
            m_backgroundMesh->makeDynamic();

            Vector2 size(rect->size());
            m_backgroundImage->composeMesh(m_backgroundMesh, Sprite::Sliced, size);

            m_imageMaterial->setTexture(gOverride, m_backgroundImage->texture());
        }
        m_dirtyBackground = false;
    }

    Canvas *canvas = Frame::canvas();
    if(m_backgroundImage) {
        Matrix4 mat(rect->worldTransform());

        const Vector3Vector &verts(m_backgroundMesh->vertices());
        Vector2 scl(rect->worldScale());
        mat[12] -= verts[0].x * scl.x;
        mat[13] -= verts[0].y * scl.y;

        uint32_t hash = rect->hash();
        Mathf::hashCombine(hash, mat[12]);
        Mathf::hashCombine(hash, mat[13]);

        m_imageMaterial->setTransform(mat, 0, hash);

        canvas->drawMesh(m_backgroundMesh, m_imageMaterial);
    } else {
        canvas->drawRect(m_frameMaterial, rect);
    }

    Widget::draw();
}
/*!
    \internal
    Applies style settings assigned to widget.
*/
void Frame::applyStyle() {
    Widget::applyStyle();

    blockSignals(true);
    // Background color
    auto it = m_styleRules.find(gCssBackgroundColor);
    if(it != m_styleRules.end()) {
        setBackgroundColor(StyleSheet::toColor(it->second.second));
    }

    // Border color
    it = m_styleRules.find(gCssBorderColor);
    if(it != m_styleRules.end()) {
        setBorderColor(StyleSheet::toColor(it->second.second));
    }

    // Border radius
    bool pixels;
    Vector4 radius(corners());
    radius = styleBlock4Length(gCssBorderRadius, radius, pixels);

    radius.x = styleLength("border-top-left-radius", radius.x, pixels);
    radius.y = styleLength("border-top-right-radius", radius.y, pixels);
    radius.z = styleLength("border-bottom-left-radius", radius.z, pixels);
    radius.w = styleLength("border-bottom-right-radius", radius.w, pixels);

    setCorners(radius);

    blockSignals(false);
}
/*!
    Returns the corners radiuses of the frame.
*/
Vector4 Frame::corners() const {
    return m_borderRadius;
}
/*!
    Sets the \a corners radiuses of the frame.
*/
void Frame::setCorners(const Vector4 &corners) {
    m_borderRadius = corners;
    if(m_frameMaterial) {
        RectTransform *rect = rectTransform();
        if(rect) {
            Vector4 normCorners(m_borderRadius / rect->size().y);
            m_frameMaterial->setVector4(gBorderRadius, &normCorners);
        }
    }
    repaint();

#ifdef SHARED_DEFINE
    if(!isSubWidget() && !isSignalsBlocked()) {
        updateStyleProperty(gCssBorderRadius, m_borderRadius.v, 4);
    }
#endif
}
/*!
    Returns the color of the frame to be drawn.
*/
Vector4 Frame::backgroundColor() const {
    return m_backgroundColor;
}
/*!
    Changes the \a color of the frame to be drawn.
*/
void Frame::setBackgroundColor(const Vector4 &color) {
    m_backgroundColor = color;
    if(m_frameMaterial) {
        m_frameMaterial->setVector4(gBackgroundColor, &m_backgroundColor);
    }

    if(m_frameMaterial) {
        m_frameMaterial->setVector4(gColor, &m_backgroundColor);
    }
    repaint();

#ifdef SHARED_DEFINE
    if(!isSubWidget() && !isSignalsBlocked()) {
        StyleSheet::setStyleProperty(this, gCssBackgroundColor, StyleSheet::toColor(m_backgroundColor));
    }
#endif
}
/*!
    Returns border color of the frame.
*/
Vector4 Frame::borderColor() const {
    return m_borderColor;
}
/*!
    Sets the border \a color of the frame.
*/
void Frame::setBorderColor(const Vector4 &color) {
    m_borderColor = color;

    if(m_frameMaterial) {
        m_frameMaterial->setVector4(gBorderColor, &color);
    }
    repaint();

#ifdef SHARED_DEFINE
    if(!isSubWidget() && !isSignalsBlocked()) {
        StyleSheet::setStyleProperty(this, gCssBorderColor, StyleSheet::toColor(m_borderColor));
    }
#endif
}
/*!
    Returns background image.
*/
Sprite *Frame::backgroundImage() const {
    return m_backgroundImage;
}
/*!
    Sets background \a image.
*/
void Frame::setBackgroundImage(Sprite *image) {
    if(m_backgroundImage != image) {
        m_backgroundImage = image;

        m_dirtyBackground = true;
        repaint();
    }
}
/*!
    Callback method called when the \a size of the frame changed.
    Updates material properties based on corner radius and border width.
*/
void Frame::boundChanged(const Vector2 &size) {
    Widget::boundChanged(size);

    if(m_frameMaterial) {
        Vector4 normCorners(m_borderRadius / size.y);
        m_frameMaterial->setVector4(gBorderRadius, &normCorners);

        Vector4 normBorders(rectTransform()->border() / size.y);
        m_frameMaterial->setVector4(gBorderWidth, &normBorders);
        repaint();
    }
}
