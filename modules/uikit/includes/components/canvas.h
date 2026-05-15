#ifndef CANVAS_H
#define CANVAS_H

#include <component.h>
#include <uikit.h>

class CommandBuffer;
class RectTransform;
class RenderTarget;
class Texture;
class Mesh;
class MaterialInstance;

class UIKIT_EXPORT Canvas : public Component {
    A_OBJECT(Canvas, Component, Components/UI)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    Canvas();

    void update(const Vector2 &pos);

    void draw(CommandBuffer *buffer);

    void drawRect(MaterialInstance *material, RectTransform *rect);

    void drawMesh(Mesh *mesh, MaterialInstance *material);

    void setSize(int width, int height);

    RectTransform *rectTransform();
    void setRectTransform(RectTransform *transform);

    void setClipRegion(const Vector4 &rect);
    void disableClip();

private:
    void composeComponent() override;

private:
    RenderTarget *m_target;

    Texture *m_texture;

    RectTransform *m_transform;

    CommandBuffer *m_buffer;

};

#endif // CANVAS_H
