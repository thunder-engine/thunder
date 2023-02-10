#include "resources/computebuffergl.h"

#include "agl.h"

ComputeBufferGL::ComputeBufferGL() :
        m_ssbo(0) {

}

uint32_t ComputeBufferGL::nativeHandle() {
    switch(state()) {
        case Unloading: {
            glDeleteBuffers(1, &m_ssbo);
            m_ssbo = 0;

            switchState(ToBeDeleted);
        } break;
        case ToBeUpdated: {
            updateBuffer();
            switchState(Ready);
        } break;
        default: break;
    }

    return m_ssbo;
}

void ComputeBufferGL::updateBuffer() {
    if(m_ssbo == 0) {
        glGenBuffers(1, &m_ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, m_buffer.size(), m_buffer.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    if(m_bufferDirty) {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_buffer.size(), m_buffer.data());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        m_bufferDirty = false;
    }
}
