#ifndef FRAME_H
#define FRAME_H

#include "widget.h"

class Mesh;
class MaterialInstance;

class UIKIT_EXPORT Frame : public Widget {
    A_OBJECT(Frame, Widget, Components/UI)

    A_PROPERTIES(
        A_PROPERTY(Vector4, corners, Frame::corners, Frame::setCorners),
        A_PROPERTYEX(Vector4, backgroundColor, Frame::color, Frame::setColor, "editor=Color, css=background-color"),
        A_PROPERTYEX(Vector4, borderColor, Frame::borderColor, Frame::setBorderColor, "editor=Color, css=border-color")
    )
    A_NOMETHODS()
    A_NOENUMS()

public:
    Frame();

    Vector4 corners() const;
    void setCorners(const Vector4 &corners);

    Vector4 color() const;
    void setColor(const Vector4 &color);

    Vector4 borderColor() const;
    void setBorderColor(const Vector4 &color);

protected:
    void boundChanged(const Vector2 &size) override;

    void draw() override;

    void applyStyle() override;

protected:
    Vector4 m_borderRadius;
    Vector4 m_backgroundColor;
    Vector4 m_borderColor;

    MaterialInstance *m_material;

};

#endif // FRAME_H
