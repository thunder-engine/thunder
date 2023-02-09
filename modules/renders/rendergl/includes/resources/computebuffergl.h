#ifndef COMPUTEBUFFERGL_H
#define COMPUTEBUFFERGL_H

#include <resources/computebuffer.h>

class ComputeBufferGL : public ComputeBuffer {
    A_OVERRIDE(ComputeBufferGL, ComputeBuffer, Resources)

    A_NOPROPERTIES()
    A_NOMETHODS()

public:
    ComputeBufferGL();

    uint32_t nativeHandle();

private:
    void updateBuffer();

    uint32_t m_ssbo;

};

#endif // COMPUTEBUFFERGL_H
