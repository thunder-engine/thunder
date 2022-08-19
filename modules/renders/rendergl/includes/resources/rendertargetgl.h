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
    void readPixels(int index, int x, int y, int width, int height) override;

    bool updateBuffer(uint32_t level);
    void destroyBuffer();

    int32_t m_buffer;

};

#endif // RENDERTARGETGL_H
