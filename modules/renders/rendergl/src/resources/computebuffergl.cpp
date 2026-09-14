/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
#include "resources/computebuffergl.h"

#include "commandbuffergl.h"

#include "agl.h"

ComputeBufferGL::ComputeBufferGL() :
        m_ssbo(0) {

}

uint32_t ComputeBufferGL::nativeHandle() {
    switch(state()) {
        case ToBeUpdated: {
            updateBuffer();
            switchState(Ready);
        } break;
        case Unloading: {
            glDeleteBuffers(1, &m_ssbo);
            m_ssbo = 0;

            switchState(ToBeDeleted);
        } break;
        default: break;
    }

    return m_ssbo;
}

void ComputeBufferGL::updateBuffer() {
#ifndef THUNDER_MOBILE
    if(m_ssbo == 0) {
        glGenBuffers(1, &m_ssbo);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, m_buffer.size(), m_buffer.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        if(!name().isEmpty()) {
            CommandBufferGL::setObjectName(GL_BUFFER, m_ssbo, name());
        }
    }

    if(m_bufferDirty) {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_buffer.size(), m_buffer.data());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        m_bufferDirty = false;
    }
#endif
}
