#ifndef FRAME_H
#define FRAME_H

#include "widget.h"

class Mesh;
class MaterialInstance;

class UIKIT_EXPORT Frame : public Widget {
    A_REGISTER(Frame, Widget, Components/UI)

    A_PROPERTIES(
        A_PROPERTY(Vector4, corners, Frame::corners, Frame::setCorners),
        A_PROPERTYEX(Vector4, color, Frame::color, Frame::setColor, "editor=Color, css=background-color"),
        A_PROPERTYEX(Vector4, topColor, Frame::topColor, Frame::setTopColor, "editor=Color, css=border-top-color"),
        A_PROPERTYEX(Vector4, rightColor, Frame::rightColor, Frame::setRightColor, "editor=Color, css=border-right-color"),
        A_PROPERTYEX(Vector4, bottomColor, Frame::bottomColor, Frame::setBottomColor, "editor=Color, css=border-bottom-color"),
        A_PROPERTYEX(Vector4, leftColor, Frame::leftColor, Frame::setLeftColor, "editor=Color, css=border-left-color")
    )
    A_NOMETHODS()
    A_NOENUMS()

public:
    Frame();

    Vector4 corners() const;
    void setCorners(Vector4 corners);

    Vector4 color() const;
    void setColor(const Vector4 color);

    Vector4 topColor() const;
    void setTopColor(Vector4 color);

    Vector4 rightColor() const;
    void setRightColor(Vector4 color);

    Vector4 bottomColor() const;
    void setBottomColor(Vector4 color);

    Vector4 leftColor() const;
    void setLeftColor(Vector4 color);

    void setBorderColor(Vector4 color);

protected:
    void boundChanged(const Vector2 &bounds) override;

    void draw(CommandBuffer &buffer) override;

    void applyStyle() override;

protected:
    Vector4 m_borderRadius;
    Vector4 m_backgroundColor;
    Vector4 m_topColor;
    Vector4 m_rightColor;
    Vector4 m_bottomColor;
    Vector4 m_leftColor;

    Vector2 m_meshSize;

    Mesh *m_mesh;

    MaterialInstance *m_material;

};

#endif // FRAME_H
