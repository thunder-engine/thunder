#ifndef FRAME_H
#define FRAME_H

#include "image.h"

class ENGINE_EXPORT Frame : public Image {
    A_REGISTER(Frame, Image, Components/UI)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    Frame();

    Vector4 corners() const;
    void setCorners(Vector4 corners);

    float borderWidth() const;
    void setBorderWidth(float width);

    Vector4 borderColor() const;
    void setBorderColor(Vector4 color);

    void setMaterial(Material *material) override;

protected:
    void boundChanged(const Vector2 &bounds) override;

private:
    Vector4 m_borderColor;
    Vector4 m_cornerRadius;

    float m_borderWidth;

};

#endif // FRAME_H
