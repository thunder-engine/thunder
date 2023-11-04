#include "components/gui/frame.h"

#include "resources/material.h"

namespace {
    const char *gBorderColor = "borderColor";
    const char *gBorderWidth = "borderWidth";
    const char *gCornerRadius = "cornerRadius";
};

Frame::Frame() :
        Image(),
        m_borderColor(0.8f),
        m_cornerRadius(10.0f),
        m_borderWidth(1.0f) {

    setDrawMode(Image::Simple);
    setMaterial(Engine::loadResource<Material>(".embedded/Frame.shader"));
}

Vector4 Frame::corners() const {
    return m_cornerRadius;
}
void Frame::setCorners(Vector4 corners) {
    m_cornerRadius = corners;
    if(m_customMaterial) {
        Vector4 normCorners(m_cornerRadius / m_meshSize.y);
        m_customMaterial->setVector4(gCornerRadius, &normCorners);
    }
}

float Frame::borderWidth() const {
    return m_borderWidth;
}
void Frame::setBorderWidth(float width) {
    m_borderWidth = width;
    if(m_customMaterial) {
        float width = m_borderWidth / m_meshSize.y;
        m_customMaterial->setFloat(gBorderWidth, &width);
    }
}

Vector4 Frame::borderColor() const {
    return m_borderColor;
}
void Frame::setBorderColor(Vector4 color) {
    m_borderColor = color;
    if(m_customMaterial) {
        m_customMaterial->setVector4(gBorderColor, &m_borderColor);
    }
}

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

void Frame::boundChanged(const Vector2 &size) {
    Image::boundChanged(size);

    if(m_customMaterial) {
        Vector4 normCorners(m_cornerRadius / m_meshSize.y);
        m_customMaterial->setVector4(gCornerRadius, &normCorners);

        float width = m_borderWidth / m_meshSize.y;
        m_customMaterial->setFloat(gBorderWidth, &width);
    }
}
