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

    const char *gTopColor("topColor");
    const char *gRightColor("rightColor");
    const char *gBottomColor("bottomColor");
    const char *gLeftColor("leftColor");

    const char *gCssBackgroundColor("background-color");
    const char *gCssBorderColor("border-color");
    const char *gCssBorderTopColor("border-top-color");
    const char *gCssBorderRightColor("border-right-color");
    const char *gCssBorderBottomColor("border-bottom-color");
    const char *gCssBorderLeftColor("border-left-color");
    const char *gCssBorderRadius("border-radius");
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
        m_material(nullptr) {

    Material *material = Engine::loadResource<Material>(".embedded/Frame.shader");
    if(material) {
        m_material = material->createInstance();

        Vector4 width(0.0f);
        m_material->setVector4(gBorderWidth, &width);
        m_material->setVector4(gBorderRadius, &m_borderRadius);
        m_material->setVector4(gTopColor, &m_borderColor);
        m_material->setVector4(gRightColor, &m_borderColor);
        m_material->setVector4(gBottomColor, &m_borderColor);
        m_material->setVector4(gLeftColor, &m_borderColor);
        m_material->setVector4(gBackgroundColor, &m_backgroundColor);
    }
}
/*!
    \internal
*/
void Frame::draw() {
    if(m_material) {
        Canvas *canvas = Frame::canvas();
        canvas->drawRect(m_material, rectTransform());
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
        setColor(StyleSheet::toColor(it->second.second));
    }

    // Border color
    it = m_styleRules.find(gCssBorderColor);
    if(it != m_styleRules.end()) {
        setBorderColor(StyleSheet::toColor(it->second.second));
    }

    it = m_styleRules.find(gCssBorderTopColor);
    if(it != m_styleRules.end()) {
        Vector4 color(StyleSheet::toColor(it->second.second));
        if(m_material) {
            m_material->setVector4(gTopColor, &color);
        }
    }

    it = m_styleRules.find(gCssBorderRightColor);
    if(it != m_styleRules.end()) {
        Vector4 color(StyleSheet::toColor(it->second.second));
        if(m_material) {
            m_material->setVector4(gRightColor, &color);
        }
    }

    it = m_styleRules.find(gCssBorderBottomColor);
    if(it != m_styleRules.end()) {
        Vector4 color(StyleSheet::toColor(it->second.second));
        if(m_material) {
            m_material->setVector4(gBottomColor, &color);
        }
    }

    it = m_styleRules.find(gCssBorderLeftColor);
    if(it != m_styleRules.end()) {
        Vector4 color(StyleSheet::toColor(it->second.second));
        if(m_material) {
            m_material->setVector4(gLeftColor, &color);
        }
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
    if(m_material) {
        RectTransform *rect = rectTransform();
        if(rect) {
            Vector4 normCorners(m_borderRadius / rect->size().y);
            m_material->setVector4(gBorderRadius, &normCorners);
        }
    }

#ifdef SHARED_DEFINE
    if(!isSubWidget() && !isSignalsBlocked()) {
        updateStyleProperty(gCssBorderRadius, m_borderRadius.v, 4);
    }
#endif
}
/*!
    Returns the color of the frame to be drawn.
*/
Vector4 Frame::color() const {
    return m_backgroundColor;
}
/*!
    Changes the \a color of the frame to be drawn.
*/
void Frame::setColor(const Vector4 &color) {
    m_backgroundColor = color;
    if(m_material) {
        m_material->setVector4(gBackgroundColor, &m_backgroundColor);
    }

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

    if(m_material) {
        m_material->setVector4(gTopColor, &color);
        m_material->setVector4(gLeftColor, &color);
        m_material->setVector4(gRightColor, &color);
        m_material->setVector4(gBottomColor, &color);
    }

#ifdef SHARED_DEFINE
    if(!isSubWidget() && !isSignalsBlocked()) {
        StyleSheet::setStyleProperty(this, gCssBorderColor, StyleSheet::toColor(m_borderColor));
    }
#endif
}
/*!
    Callback method called when the \a size of the frame changed.
    Updates material properties based on corner radius and border width.
*/
void Frame::boundChanged(const Vector2 &size) {
    Widget::boundChanged(size);

    if(m_material) {
        Vector4 normCorners(m_borderRadius / size.y);
        m_material->setVector4(gBorderRadius, &normCorners);

        Vector4 normBorders(rectTransform()->border() / size.y);
        m_material->setVector4(gBorderWidth, &normBorders);
    }
}
