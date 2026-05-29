#ifndef FRAME_H
#define FRAME_H

#include "widget.h"

#include <resources/sprite.h>

class Mesh;
class MaterialInstance;

class UIKIT_EXPORT Frame : public Widget {
    A_OBJECT(Frame, Widget, Components/UI)

    A_PROPERTIES(
        A_PROPERTY(Vector4, corners, Frame::corners, Frame::setCorners),
        A_PROPERTYEX(Vector4, backgroundColor, Frame::backgroundColor, Frame::setBackgroundColor, "editor=Color, css=background-color"),
        A_PROPERTYEX(Vector4, borderColor, Frame::borderColor, Frame::setBorderColor, "editor=Color, css=border-color"),
        A_PROPERTYEX(Sprite *, backgroundImage, Frame::backgroundImage, Frame::setBackgroundImage, "editor=Asset")
    )
    A_NOMETHODS()
    A_NOENUMS()

public:
    Frame();
    ~Frame();

    Vector4 corners() const;
    void setCorners(const Vector4 &corners);

    Vector4 backgroundColor() const;
    void setBackgroundColor(const Vector4 &color);

    Vector4 borderColor() const;
    void setBorderColor(const Vector4 &color);

    Sprite *backgroundImage() const;
    void setBackgroundImage(Sprite *image);

protected:
    void boundChanged(const Vector2 &size) override;

    void draw() override;

    void applyStyle() override;

protected:
    Vector4 m_borderRadius;
    Vector4 m_backgroundColor;
    Vector4 m_borderColor;

    Sprite *m_backgroundImage;

    Mesh *m_backgroundMesh;

    MaterialInstance *m_imageMaterial;
    MaterialInstance *m_frameMaterial;

    bool m_dirtyBackground;

};

#endif // FRAME_H
