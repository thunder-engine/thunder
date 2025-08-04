#include "components/frame.h"

#include "components/recttransform.h"

#include <components/actor.h>
#include <components/spriterender.h>

#include <resources/sprite.h>
#include <resources/mesh.h>
#include <resources/material.h>
#include <resources/stylesheet.h>

#include <pipelinecontext.h>
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
};

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
        m_topColor(0.8f),
        m_rightColor(0.8f),
        m_bottomColor(0.8f),
        m_leftColor(0.8f),
        m_mesh(PipelineContext::defaultPlane()),
        m_material(nullptr) {

    Material *material = Engine::loadResource<Material>(".embedded/Frame.shader");
    if(material) {
        m_material = material->createInstance();

        m_material->setVector4(gBorderRadius, &m_borderRadius);
        m_material->setVector4(gTopColor, &m_topColor);
        m_material->setVector4(gRightColor, &m_rightColor);
        m_material->setVector4(gBottomColor, &m_bottomColor);
        m_material->setVector4(gLeftColor, &m_leftColor);
        m_material->setVector4(gBackgroundColor, &m_backgroundColor);
    }
}
/*!
    \internal
*/
void Frame::draw(CommandBuffer &buffer) {
    if(m_material) {
        RectTransform *rect = rectTransform();
        Matrix4 m(rect->worldTransform());

        Vector2 size(rect->size());
        Matrix4 s;
        s[0] = size.x;
        s[5] = size.y;
        s[12] = size.x * 0.5f;
        s[13] = size.y * 0.5f;

        m_material->setTransform(m * s);

        buffer.drawMesh(m_mesh, 0, Material::Translucent, *m_material);
    }
}
/*!
    \internal
    Applies style settings assigned to widget.
*/
void Frame::applyStyle() {
    Widget::applyStyle();

    // Background color
    auto it = m_styleRules.find("background-color");
    if(it != m_styleRules.end()) {
        setColor(StyleSheet::toColor(it->second.second));
    }

    // Border color
    it = m_styleRules.find("border-color");
    if(it != m_styleRules.end()) {
        setBorderColor(StyleSheet::toColor(it->second.second));
    }

    it = m_styleRules.find("border-top-color");
    if(it != m_styleRules.end()) {
        setTopColor(StyleSheet::toColor(it->second.second));
    }

    it = m_styleRules.find("border-right-color");
    if(it != m_styleRules.end()) {
        setRightColor(StyleSheet::toColor(it->second.second));
    }

    it = m_styleRules.find("border-bottom-color");
    if(it != m_styleRules.end()) {
        setBottomColor(StyleSheet::toColor(it->second.second));
    }

    it = m_styleRules.find("border-left-color");
    if(it != m_styleRules.end()) {
        setLeftColor(StyleSheet::toColor(it->second.second));
    }

    // Border radius
    bool pixels;
    Vector4 radius(corners());
    radius = styleBlock4Length("border-radius", radius, pixels);

    radius.x = styleLength("border-top-left-radius", radius.x, pixels);
    radius.y = styleLength("border-top-right-radius", radius.y, pixels);
    radius.z = styleLength("border-bottom-left-radius", radius.z, pixels);
    radius.w = styleLength("border-bottom-right-radius", radius.w, pixels);

    setCorners(radius);
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
void Frame::setCorners(Vector4 corners) {
    m_borderRadius = corners;
    if(m_material) {
        RectTransform *rect = rectTransform();
        Vector4 normCorners(m_borderRadius / rect->size().y);
        m_material->setVector4(gBorderRadius, &normCorners);
    }
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
void Frame::setColor(const Vector4 color) {
    m_backgroundColor = color;
    if(m_material) {
        m_material->setVector4(gBackgroundColor, &m_backgroundColor);
    }
}
/*!
    Returns the top border color of the frame.
*/
Vector4 Frame::topColor() const {
    return m_topColor;
}
/*!
    Sets the top border \a color of the frame.
*/
void Frame::setTopColor(Vector4 color) {
    m_topColor = color;
    if(m_material) {
        m_material->setVector4(gTopColor, &m_topColor);
    }
}
/*!
    Returns the right border color of the frame.
*/
Vector4 Frame::rightColor() const {
    return m_rightColor;
}
/*!
    Sets the right border \a color of the frame.
*/
void Frame::setRightColor(Vector4 color) {
    m_rightColor = color;
    if(m_material) {
        m_material->setVector4(gRightColor, &m_rightColor);
    }
}
/*!
    Returns the bottom border color of the frame.
*/
Vector4 Frame::bottomColor() const {
    return m_bottomColor;
}
/*!
    Sets the bottom border \a color of the frame.
*/
void Frame::setBottomColor(Vector4 color) {
    m_bottomColor = color;
    if(m_material) {
        m_material->setVector4(gBottomColor, &m_bottomColor);
    }
}
/*!
    Returns the left border color of the frame.
*/
Vector4 Frame::leftColor() const {
    return m_leftColor;
}
/*!
    Sets the left border \a color of the frame.
*/
void Frame::setLeftColor(Vector4 color) {
    m_leftColor = color;
    if(m_material) {
        m_material->setVector4(gLeftColor, &m_leftColor);
    }
}
/*!
    Sets the border \a color of the frame.
*/
void Frame::setBorderColor(Vector4 color) {
    setTopColor(color);
    setRightColor(color);
    setBottomColor(color);
    setLeftColor(color);
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
