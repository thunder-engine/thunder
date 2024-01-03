#include "components/gui/frame.h"

#include "resources/material.h"

namespace {
    const char *gBorderColor = "borderColor";
    const char *gBorderWidth = "borderWidth";
    const char *gCornerRadius = "cornerRadius";
};

/*!
    \class Frame
    \brief The Frame class represents a graphical frame or border with customizable corners, border width, and border color.
    \inmodule Gui
*/

Frame::Frame() :
        Image(),
        m_borderColor(0.8f),
        m_cornerRadius(10.0f),
        m_borderWidth(1.0f) {

    setDrawMode(Image::Simple);
    setMaterial(Engine::loadResource<Material>(".embedded/Frame.shader"));
}
/*!
    Returns the corners radiuses of the frame.
*/
Vector4 Frame::corners() const {
    return m_cornerRadius;
}
/*!
    Sets the \a corners radiuses of the frame.
*/
void Frame::setCorners(Vector4 corners) {
    m_cornerRadius = corners;
    if(m_customMaterial) {
        Vector4 normCorners(m_cornerRadius / m_meshSize.y);
        m_customMaterial->setVector4(gCornerRadius, &normCorners);
    }
}
/*!
    Returns the border width of the frame.
*/
float Frame::borderWidth() const {
    return m_borderWidth;
}
/*!
    Sets the border \a width of the frame.
*/
void Frame::setBorderWidth(float width) {
    m_borderWidth = width;
    if(m_customMaterial) {
        float width = m_borderWidth / m_meshSize.y;
        m_customMaterial->setFloat(gBorderWidth, &width);
    }
}
/*!
    Returns the border color of the frame.
*/
Vector4 Frame::borderColor() const {
    return m_borderColor;
}
/*!
    Sets the border \a color of the frame.
*/
void Frame::setBorderColor(Vector4 color) {
    m_borderColor = color;
    if(m_customMaterial) {
        m_customMaterial->setVector4(gBorderColor, &m_borderColor);
    }
}
/*!
    Sets the \a material for the frame and updates material properties based on corner radius, border width, and border color.
*/
void Frame::setMaterial(Material *material) {
    Image::setMaterial(material);
    if(m_customMaterial) {
        Vector4 normCorners(m_cornerRadius / m_meshSize.y);
        m_customMaterial->setVector4(gCornerRadius, &normCorners);

        float width = m_borderWidth / m_meshSize.y;
        m_customMaterial->setFloat(gBorderWidth, &width);

        m_customMaterial->setVector4(gBorderColor, &m_borderColor);
    }
}
/*!
    Callback method called when the \a bounds of the frame change. Updates material properties based on corner radius and border width.
*/
void Frame::boundChanged(const Vector2 &bounds) {
    Image::boundChanged(bounds);

    if(m_customMaterial) {
        Vector4 normCorners(m_cornerRadius / m_meshSize.y);
        m_customMaterial->setVector4(gCornerRadius, &normCorners);

        float width = m_borderWidth / m_meshSize.y;
        m_customMaterial->setFloat(gBorderWidth, &width);
    }
}
