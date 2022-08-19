#ifndef RENDERTARGET_H
#define RENDERTARGET_H

#include "resource.h"

class RenderTargetPrivate;

class Texture;

class ENGINE_EXPORT RenderTarget : public Resource {
    A_REGISTER(RenderTarget, Resource, Resources)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    RenderTarget();
    ~RenderTarget();

    uint32_t colorAttachmentCount() const;

    Texture *colorAttachment(uint32_t index) const;
    virtual uint32_t setColorAttachment(uint32_t index, Texture *texture);

    Texture *depthAttachment() const;
    virtual void setDepthAttachment(Texture *texture);

    virtual void readPixels(int index, int x, int y, int width, int height);

protected:
    void makeNative();
    bool isNative() const;

    void switchState(ResourceState state) override;
    bool isUnloadable() override;

private:
    RenderTargetPrivate *p_ptr;

};

#endif // RENDERTARGET_H
