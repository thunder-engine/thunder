#include "resources/computebuffer.h"

ComputeBuffer::ComputeBuffer() :
        m_bufferDirty(false) {

}

ComputeBuffer::~ComputeBuffer() {

}

uint32_t ComputeBuffer::count() const {
    return 0;
}

uint32_t ComputeBuffer::stride() const {
    return 0;
}
void ComputeBuffer::setStride(uint32_t stride) {
    A_UNUSED(stride);
}

ByteArray ComputeBuffer::data() const {
    return m_buffer;
}
void ComputeBuffer::setData(const ByteArray &data) {
    m_buffer = data;
    m_bufferDirty = true;
}
