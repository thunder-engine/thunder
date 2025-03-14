#ifndef COMPUTEBUFFERMT_H
#define COMPUTEBUFFERMT_H

#include <resources/computebuffer.h>

#include "wrappermt.h"

class ComputeBufferMt : public ComputeBuffer {
    A_OVERRIDE(ComputeBufferMt, ComputeBuffer, Resources)

    A_NOPROPERTIES()
    A_NOMETHODS()

public:
    ComputeBufferMt();

    MTL::Buffer *nativeHandle();

private:
    MTL::Buffer *m_buffer;

};

#endif // COMPUTEBUFFERMT_H
