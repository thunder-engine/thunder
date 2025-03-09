#include "resources/computebuffermt.h"

#include "commandbuffermt.h"

ComputeBufferMt::ComputeBufferMt() :
        m_buffer(nullptr) {

}

MTL::Buffer *ComputeBufferMt::nativeHandle() {
    return m_buffer;
}
