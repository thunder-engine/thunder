#ifndef RENDERTARGET_H
#define RENDERTARGET_H

#include "resource.h"

class Texture;

class ENGINE_EXPORT RenderTarget : public Resource {
    A_OBJECT(RenderTarget, Resource, Resources)

    A_NOPROPERTIES()
    A_NOMETHODS()

    enum Flags {
        ClearColor = (1<<0),
        ClearDepth = (1<<1),
        Atlas = (1<<2)
    };

    A_NOENUMS()

public:
    RenderTarget();
    ~RenderTarget();

    virtual uint32_t colorAttachmentCount() const;

    Texture *colorAttachment(uint32_t index) const;
    virtual uint32_t setColorAttachment(uint32_t index, Texture *texture);

    Texture *depthAttachment() const;
    virtual void setDepthAttachment(Texture *texture);

    int flags() const;
    void setFlags(int flags);

    const Vector4 &clearColor() const;
    void setClearColor(const Vector4 &color);

    void renderArea(int32_t &x, int32_t &y, int32_t &width, int32_t &height) const;
    void setRenderArea(int32_t x, int32_t y, int32_t width, int32_t height);

    int32_t tileIndex() const;
    void setTileIndex(int32_t index);

protected:
    void switchState(Resource::State state) override;
    bool isUnloadable() override;

private:
    std::vector<Texture *> m_color;

    Vector4 m_clearColor;

    Texture *m_depth;

    int m_flags;

    int m_clearX;

    int m_clearY;

    int m_clearWidth;

    int m_clearHeigh;

    int m_currentTile;

};

#endif // RENDERTARGET_H
