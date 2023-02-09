#include "resources/meshgl.h"

#include "agl.h"

#include "commandbuffergl.h"

MeshGL::MeshGL() :
    m_triangles(0),
    m_uv0(0),
    m_uv1(0),
    m_normals(0),
    m_tangents(0),
    m_vertices(0),
    m_colors(0),
    m_weights(0),
    m_bones(0),
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
    glEnableVertexAttribArray(VERTEX_ATRIB);
    glVertexAttribPointer(VERTEX_ATRIB, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    if(!normals().empty()) {
        glBindBuffer(GL_ARRAY_BUFFER, m_normals);
        glEnableVertexAttribArray(NORMAL_ATRIB);
        glVertexAttribPointer(NORMAL_ATRIB, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    }
    if(!tangents().empty()) {
        glBindBuffer(GL_ARRAY_BUFFER, m_tangents);
        glEnableVertexAttribArray(TANGENT_ATRIB);
        glVertexAttribPointer(TANGENT_ATRIB, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    }
    if(!uv0().empty()) {
        glBindBuffer(GL_ARRAY_BUFFER, m_uv0);
        glEnableVertexAttribArray(UV0_ATRIB);
        glVertexAttribPointer(UV0_ATRIB, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    }
    if(!uv1().empty()) {
        glBindBuffer(GL_ARRAY_BUFFER, m_uv1);
        glEnableVertexAttribArray(UV1_ATRIB);
        glVertexAttribPointer(UV1_ATRIB, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    }
    if(!colors().empty()) {
        //glBindBuffer(GL_ARRAY_BUFFER, m_colors[lod]);
        //glEnableVertexAttribArray(COLOR_ATRIB);
        //glVertexAttribPointer(COLOR_ATRIB, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    }
    if(!weights().empty()) {
        glBindBuffer(GL_ARRAY_BUFFER, m_bones);
        glEnableVertexAttribArray(BONES_ATRIB);
        glVertexAttribPointer(BONES_ATRIB, 4, GL_FLOAT, GL_FALSE, 0, nullptr);

        glBindBuffer(GL_ARRAY_BUFFER, m_weights);
        glEnableVertexAttribArray(WEIGHTS_ATRIB);
        glVertexAttribPointer(WEIGHTS_ATRIB, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
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
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3) * vCount, vertices().data(), usage);
    }

    if(!normals().empty()) {
        if(m_normals == 0) {
            glGenBuffers(1, &m_normals);
        }
        glBindBuffer(GL_ARRAY_BUFFER, m_normals);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3) * vCount, normals().data(), usage);
    }
    if(!tangents().empty()) {
        if(m_tangents == 0) {
            glGenBuffers(1, &m_tangents);
        }
        glBindBuffer(GL_ARRAY_BUFFER, m_tangents);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3) * vCount, tangents().data(), usage);
    }
    if(!uv0().empty()) {
        if(m_uv0 == 0) {
            glGenBuffers(1, &m_uv0);
        }
        glBindBuffer(GL_ARRAY_BUFFER, m_uv0);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vector2) * vCount, uv0().data(), usage);
    }
    if(!uv1().empty()) {
        if(m_uv1 == 0) {
            glGenBuffers(1, &m_uv1);
        }
        glBindBuffer(GL_ARRAY_BUFFER, m_uv1);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vector2) * vCount, uv1().data(), usage);
    }
    if(!weights().empty()) {
        if(m_weights == 0) {
            glGenBuffers(1, &m_weights);
        }
        glBindBuffer(GL_ARRAY_BUFFER, m_weights);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vector4) * vCount, weights().data(), usage);
    }
    if(!bones().empty()) {
        if(m_bones == 0) {
            glGenBuffers(1, &m_bones);
        }
        glBindBuffer(GL_ARRAY_BUFFER, m_bones);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vector4) * vCount, bones().data(), (dynamic) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
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

    if(m_colors != 0) {
        glDeleteBuffers(1, &m_colors);
        m_colors = 0;
    }
    if(m_normals != 0) {
        glDeleteBuffers(1, &m_normals);
        m_normals = 0;
    }
    if(m_tangents != 0) {
        glDeleteBuffers(1, &m_tangents);
        m_tangents = 0;
    }
    if(m_uv0 != 0) {
        glDeleteBuffers(1, &m_uv0);
        m_uv0 = 0;
    }
    if(m_uv1 != 0) {
        glDeleteBuffers(1, &m_uv1);
        m_uv1 = 0;
    }

    if(m_weights != 0) {
        glDeleteBuffers(1, &m_weights);
        m_weights = 0;
    }
    if(m_bones != 0) {
        glDeleteBuffers(1, &m_bones);
        m_bones = 0;

    }

    if(m_instanceBuffer != 0) {
        glDeleteBuffers(1, &m_instanceBuffer);
        m_instanceBuffer = 0;
    }
}

uint32_t MeshGL::instance() const {
    return m_instanceBuffer;
}
