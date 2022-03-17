#ifndef RENDERTARGETGL_H
#define RENDERTARGETGL_H

#include <resources/rendertarget.h>

class RenderTargetGL : public RenderTarget {
    A_OVERRIDE(RenderTargetGL, RenderTarget, Resources)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    RenderTargetGL();

    void bindBuffer(uint32_t level);

    uint32_t nativeHandle() const;
    void setNativeHandle(uint32_t id);

private:
    bool updateBuffer(uint32_t level);
    void destroyBuffer();

    int32_t m_Buffer;

};

#endif // RENDERTARGETGL_H
