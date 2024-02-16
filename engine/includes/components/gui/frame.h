#ifndef FRAME_H
#define FRAME_H

#include "widget.h"

class Mesh;
class MaterialInstance;

class ENGINE_EXPORT Frame : public Widget {
    A_REGISTER(Frame, Widget, Components/UI)

    A_PROPERTIES(
        A_PROPERTY(Vector4, corners, Frame::corners, Frame::setCorners),
        A_PROPERTY(float, borderWidth, Frame::borderWidth, Frame::setBorderWidth),
        A_PROPERTYEX(Vector4, color, Frame::color, Frame::setColor, "editor=Color"),
        A_PROPERTYEX(Vector4, borderColor, Frame::borderColor, Frame::setBorderColor, "editor=Color")
    )
    A_NOMETHODS()
    A_NOENUMS()

public:
    Frame();

    Vector4 corners() const;
    void setCorners(Vector4 corners);

    float borderWidth() const;
    void setBorderWidth(float width);

    Vector4 color() const;
    void setColor(const Vector4 color);

    Vector4 borderColor() const;
    void setBorderColor(Vector4 color);

protected:
    void boundChanged(const Vector2 &bounds) override;

    void draw(CommandBuffer &buffer) override;

protected:
    Vector4 m_frameColor;
    Vector4 m_borderColor;
    Vector4 m_cornerRadius;

    Vector2 m_meshSize;

    Mesh *m_mesh;

    MaterialInstance *m_material;

    float m_borderWidth;

};

#endif // FRAME_H
