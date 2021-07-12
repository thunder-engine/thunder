#ifndef RENDERTARGET_H
#define RENDERTARGET_H

#include "resource.h"

class RenderTargetPrivate;

class Texture;

class NEXT_LIBRARY_EXPORT RenderTarget : public Resource {
    A_REGISTER(RenderTarget, Resource, Resources)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    RenderTarget();
    ~RenderTarget();

    uint32_t colorAttachmentCount() const;

    Texture *colorAttachment(uint32_t index) const;
    uint32_t setColorAttachment(uint32_t index, Texture *texture);

    Texture *depthAttachment() const;
    void setDepthAttachment(Texture *texture);

private:
    RenderTargetPrivate *p_ptr;

};

#endif // RENDERTARGET_H
