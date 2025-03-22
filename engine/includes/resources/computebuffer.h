#ifndef COMPUTEBUFFER_H
#define COMPUTEBUFFER_H

#include "resource.h"

class ENGINE_EXPORT ComputeBuffer : public Resource {
    A_OBJECT(ComputeBuffer, Resource, Resources)

public:
    ComputeBuffer();
    ~ComputeBuffer();

    uint32_t count() const;

    uint32_t stride() const;
    void setStride(uint32_t stride);

    virtual ByteArray data() const;
    void setData(const ByteArray &data);

protected:
    ByteArray m_buffer;

    bool m_bufferDirty;

};

#endif // COMPUTEBUFFER_H
