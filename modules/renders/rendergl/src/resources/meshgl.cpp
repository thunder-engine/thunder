#include "resources/meshgl.h"

#include "agl.h"

#include "commandbuffergl.h"

MeshGL::MeshGL() :
    m_triangles(0),
    m_vertices(0),
    m_instanceBuffer(0) {
}

void MeshGL::bindVao(CommandBufferGL *buffer) {
    switch(state()) {
        case ToBeUpdated: {
            updateVbo(buffer);

            switchState(Ready);
        } break;
        case Ready: break;
        case Unloading: {
            destroyVbo();
            destroyVao(buffer);

            switchState(ToBeDeleted);
            return;
        }
        default: return;
    }

    uint32_t *id = nullptr;
    for(auto &it : m_vao) {
        if(it->buffer == buffer) {
            if(it->dirty) {
                id = &(it->vao);
                glDeleteVertexArrays(1, id);
                break;
            } else if(glIsVertexArray(it->vao)) {
                glBindVertexArray(it->vao);
                return;
            }
        }
    }
    if(id == nullptr) {
        VaoStruct *vao = new VaoStruct;
        vao->buffer = buffer;
        vao->dirty = false;
        id = &(vao->vao);
        m_vao.push_back(vao);
    }

    glGenVertexArrays(1, id);
    glBindVertexArray(*id);

    updateVao();
}

void MeshGL::updateVao() {
    // indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_triangles);
    // vertices
    glBindBuffer(GL_ARRAY_BUFFER, m_vertices);

    uint32_t vCount = vertices().size();
    glEnableVertexAttribArray(VERTEX_ATRIB);
    glVertexAttribPointer(VERTEX_ATRIB, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    size_t offset = sizeof(Vector3) * vCount;

    // The order is matter and must be the same with MeshGL::updateVbo attributes
    if(!uv0().empty()) {
        glEnableVertexAttribArray(UV0_ATRIB);
        glVertexAttribPointer(UV0_ATRIB, 2, GL_FLOAT, GL_FALSE, 0, (void *)offset);
        offset += sizeof(Vector2) * vCount;
    }
    if(!colors().empty()) {
        glEnableVertexAttribArray(COLOR_ATRIB);
        glVertexAttribPointer(COLOR_ATRIB, 4, GL_FLOAT, GL_FALSE, 0, (void *)offset);
        offset += sizeof(Vector4) * vCount;
    }
    if(!normals().empty()) {
        glEnableVertexAttribArray(NORMAL_ATRIB);
        glVertexAttribPointer(NORMAL_ATRIB, 3, GL_FLOAT, GL_FALSE, 0, (void *)offset);
        offset += sizeof(Vector3) * vCount;
    }
    if(!tangents().empty()) {
        glEnableVertexAttribArray(TANGENT_ATRIB);
        glVertexAttribPointer(TANGENT_ATRIB, 3, GL_FLOAT, GL_FALSE, 0, (void *)offset);
        offset += sizeof(Vector3) * vCount;
    }
    if(!weights().empty()) {
        glEnableVertexAttribArray(BONES_ATRIB);
        glVertexAttribPointer(BONES_ATRIB, 4, GL_FLOAT, GL_FALSE, 0, (void *)offset);
        offset += sizeof(Vector4) * vCount;

        glEnableVertexAttribArray(WEIGHTS_ATRIB);
        glVertexAttribPointer(WEIGHTS_ATRIB, 4, GL_FLOAT, GL_FALSE, 0, (void *)offset);
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_instanceBuffer);
    for(uint32_t i = 0; i < 4; i++) {
        glEnableVertexAttribArray(INSTANCE_ATRIB + i);
        glVertexAttribPointer(INSTANCE_ATRIB + i, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix4), reinterpret_cast<void *>(i * sizeof(Vector4)));
        glVertexAttribDivisor(INSTANCE_ATRIB + i, 1);
    }
}

void MeshGL::updateVbo(CommandBufferGL *buffer) {
    if(!m_instanceBuffer) {
        glGenBuffers(1, &m_instanceBuffer);
    }

    bool dynamic = isDynamic();
    uint32_t usage = (dynamic) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
    uint32_t vCount = vertices().size();

    if(!indices().empty()) {
        if(m_triangles == 0) {
            glGenBuffers(1, &m_triangles);
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_triangles);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * indices().size(), indices().data(), usage);
    }

    if(!vertices().empty()) {
        if(m_vertices == 0) {
            glGenBuffers(1, &m_vertices);
        }
        glBindBuffer(GL_ARRAY_BUFFER, m_vertices);

        uint32_t size = sizeof(Vector3) * vCount;
        if(!uv0().empty()) size += sizeof(Vector2) * vCount;
        if(!colors().empty()) size += sizeof(Vector4) * vCount;
        if(!normals().empty()) size += sizeof(Vector3) * vCount;
        if(!tangents().empty()) size += sizeof(Vector3) * vCount;
        if(!weights().empty()) size += sizeof(Vector4) * vCount;
        if(!bones().empty()) size += sizeof(Vector4) * vCount;

        glBufferData(GL_ARRAY_BUFFER, size, vertices().data(), usage);
        size_t offset = sizeof(Vector3) * vCount;

        if(!uv0().empty()) {
            size = sizeof(Vector2) * vCount;
            glBufferSubData(GL_ARRAY_BUFFER, offset, size, uv0().data());
            offset += size;
        }
        if(!colors().empty()) {
            size = sizeof(Vector4) * vCount;
            glBufferSubData(GL_ARRAY_BUFFER, offset, size, colors().data());
            offset += size;
        }
        if(!normals().empty()) {
            size = sizeof(Vector3) * vCount;
            glBufferSubData(GL_ARRAY_BUFFER, offset, size, normals().data());
            offset += size;
        }
        if(!tangents().empty()) {
            size = sizeof(Vector3) * vCount;
            glBufferSubData(GL_ARRAY_BUFFER, offset, size, tangents().data());
            offset += size;
        }
        if(!weights().empty()) {
            size = sizeof(Vector4) * vCount;
            glBufferSubData(GL_ARRAY_BUFFER, offset, size, weights().data());
            offset += size;
        }
        if(!bones().empty()) {
            size = sizeof(Vector4) * vCount;
            glBufferSubData(GL_ARRAY_BUFFER, offset, size, bones().data());
        }
    }

    if(dynamic) {
        for(auto &it : m_vao) {
            if(it->buffer == buffer) {
                it->dirty = true;
            }
        }
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void MeshGL::destroyVao(CommandBufferGL *buffer) {
    for(auto &it : m_vao) {
        if(it->buffer == buffer) {
            glDeleteVertexArrays(1, &(it->vao));
        }
    }
}

void MeshGL::destroyVbo() {
    if(m_vertices == 0 && m_triangles == 0) {
        return;
    }
    glDeleteBuffers(1, &m_vertices);
    m_vertices = 0;

    glDeleteBuffers(1, &m_triangles);
    m_triangles = 0;

    if(m_instanceBuffer != 0) {
        glDeleteBuffers(1, &m_instanceBuffer);
        m_instanceBuffer = 0;
    }
}

uint32_t MeshGL::instance() const {
    return m_instanceBuffer;
}
