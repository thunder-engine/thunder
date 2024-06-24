#ifndef RENDERTARGET_H
#define RENDERTARGET_H

#include "resource.h"

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

protected:
    void makeNative();
    bool isNative() const;

    void switchState(Resource::State state) override;
    bool isUnloadable() override;

private:
    vector<Texture *> m_color;

    Texture *m_depth;

    bool m_native = false;

};

#endif // RENDERTARGET_H
