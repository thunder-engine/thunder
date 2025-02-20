#ifndef RENDERTARGET_H
#define RENDERTARGET_H

#include "resource.h"

class Texture;

class ENGINE_EXPORT RenderTarget : public Resource {
    A_REGISTER(RenderTarget, Resource, Resources)

    A_NOPROPERTIES()
    A_NOMETHODS()

    enum ClearFlags {
        DoNothing = 0,
        ClearColor,
        ClearDepth
    };

    A_NOENUMS()

public:
    RenderTarget();
    ~RenderTarget();

    uint32_t colorAttachmentCount() const;

    Texture *colorAttachment(uint32_t index) const;
    virtual uint32_t setColorAttachment(uint32_t index, Texture *texture);

    Texture *depthAttachment() const;
    virtual void setDepthAttachment(Texture *texture);

    int clearFlags() const;
    void setClearFlags(int flags);

    void clearRegion(int32_t &x, int32_t &y, int32_t &width, int32_t &height) const;
    void setClearRegion(int32_t x, int32_t y, int32_t width, int32_t height);

protected:
    void makeNative();
    bool isNative() const;

    void switchState(Resource::State state) override;
    bool isUnloadable() override;

private:
    std::vector<Texture *> m_color;

    Texture *m_depth;

    int m_clearFlags;

    int m_clearX;

    int m_clearY;

    int m_clearWidth;

    int m_clearHeigh;

    bool m_native;

};

#endif // RENDERTARGET_H
