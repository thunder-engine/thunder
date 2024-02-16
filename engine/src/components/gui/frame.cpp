#include "components/gui/frame.h"

#include "components/actor.h"
#include "components/spriterender.h"
#include "components/gui/recttransform.h"

#include "resources/sprite.h"
#include "resources/mesh.h"
#include "resources/material.h"

#include "commandbuffer.h"

namespace {
    const char *gFrameColor = "frameColor";
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
        Widget(),
        m_frameColor(1.0f, 1.0f, 1.0f, 0.5f),
        m_borderColor(0.8f),
        m_cornerRadius(0.0f),
        m_mesh(Engine::objectCreate<Mesh>("")),
        m_material(nullptr),
        m_borderWidth(1.0f) {

    Material *material = Engine::loadResource<Material>(".embedded/Frame.shader");
    if(material) {
        m_material = material->createInstance();

        Vector4 normCorners(m_cornerRadius / m_meshSize.y);
        m_material->setVector4(gCornerRadius, &normCorners);

        float width = m_borderWidth / m_meshSize.y;
        m_material->setFloat(gBorderWidth, &width);

        m_material->setVector4(gBorderColor, &m_borderColor);
        m_material->setVector4(gFrameColor, &m_frameColor);
    }
}
/*!
    \internal
*/
void Frame::draw(CommandBuffer &buffer) {
    if(m_mesh) {
        Vector3Vector &verts = m_mesh->vertices();
        if(!verts.empty()) {
            Matrix4 mat(rectTransform()->worldTransform());
            mat[12] -= verts[0].x;
            mat[13] -= verts[0].y;

            buffer.setObjectId(actor()->uuid());
            buffer.setMaterialId(m_material->material()->uuid());

            buffer.drawMesh(mat, m_mesh, 0, CommandBuffer::UI, m_material);
        }
    }
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
    if(m_material) {
        Vector4 normCorners(m_cornerRadius / m_meshSize.y);
        m_material->setVector4(gCornerRadius, &normCorners);
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
    if(m_material) {
        float width = m_borderWidth / m_meshSize.y;
        m_material->setFloat(gBorderWidth, &width);
    }
}
/*!
    Returns the color of the frame to be drawn.
*/
Vector4 Frame::color() const {
    return m_frameColor;
}
/*!
    Changes the \a color of the frame to be drawn.
*/
void Frame::setColor(const Vector4 color) {
    m_frameColor = color;
    if(m_material) {
        m_material->setVector4(gFrameColor, &m_frameColor);
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
    if(m_material) {
        m_material->setVector4(gBorderColor, &m_borderColor);
    }
}
/*!
    Callback method called when the \a bounds of the frame change.
    Updates material properties based on corner radius and border width.
*/
void Frame::boundChanged(const Vector2 &bounds) {
    m_meshSize = bounds;
    SpriteRender::composeMesh(nullptr, 0, m_mesh, m_meshSize, SpriteRender::Simple, false, 100.0f);

    if(m_material) {
        Vector4 normCorners(m_cornerRadius / m_meshSize.y);
        m_material->setVector4(gCornerRadius, &normCorners);

        float width = m_borderWidth / m_meshSize.y;
        m_material->setFloat(gBorderWidth, &width);
    }
}
