#include "resources/meshgl.h"

#include "agl.h"

#include "commandbuffergl.h"

MeshGL::MeshGL() :
        m_InstanceBuffer(0) {
}

void MeshGL::bindVao(CommandBufferGL *buffer, uint32_t lod) {
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
    for(auto &it : m_Vao[lod]) {
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
        m_Vao[lod].push_back(vao);
    }

    glGenVertexArrays(1, id);
    glBindVertexArray(*id);

    updateVao(lod);
}

void MeshGL::updateVao(uint32_t lod) {
    // indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_triangles[lod]);
    // vertices
    glBindBuffer(GL_ARRAY_BUFFER, m_vertices[lod]);
    glEnableVertexAttribArray(VERTEX_ATRIB);
    glVertexAttribPointer(VERTEX_ATRIB, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    uint8_t flag = flags();

    if(flag & Mesh::Normals) {
        glBindBuffer(GL_ARRAY_BUFFER, m_normals[lod]);
        glEnableVertexAttribArray(NORMAL_ATRIB);
        glVertexAttribPointer(NORMAL_ATRIB, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    }
    if(flag & Mesh::Tangents) {
        glBindBuffer(GL_ARRAY_BUFFER, m_tangents[lod]);
        glEnableVertexAttribArray(TANGENT_ATRIB);
        glVertexAttribPointer(TANGENT_ATRIB, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    }
    if(flag & Mesh::Uv0) {
        glBindBuffer(GL_ARRAY_BUFFER, m_uv0[lod]);
        glEnableVertexAttribArray(UV0_ATRIB);
        glVertexAttribPointer(UV0_ATRIB, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    }
    if(flag & Mesh::Color) {
        //glBindBuffer(GL_ARRAY_BUFFER, m_colors[lod]);
        //glEnableVertexAttribArray(COLOR_ATRIB);
        //glVertexAttribPointer(COLOR_ATRIB, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    }
    if(flag & Mesh::Skinned) {
        glBindBuffer(GL_ARRAY_BUFFER, m_bones[lod]);
        glEnableVertexAttribArray(BONES_ATRIB);
        glVertexAttribPointer(BONES_ATRIB, 4, GL_FLOAT, GL_FALSE, 0, nullptr);

        glBindBuffer(GL_ARRAY_BUFFER, m_weights[lod]);
        glEnableVertexAttribArray(WEIGHTS_ATRIB);
        glVertexAttribPointer(WEIGHTS_ATRIB, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_InstanceBuffer);
    for(uint32_t i = 0; i < 4; i++) {
        glEnableVertexAttribArray(INSTANCE_ATRIB + i);
        glVertexAttribPointer(INSTANCE_ATRIB + i, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix4), reinterpret_cast<void *>(i * sizeof(Vector4)));
        glVertexAttribDivisor(INSTANCE_ATRIB + i, 1);
    }
}

void MeshGL::updateVbo(CommandBufferGL *buffer) {
    if(!m_InstanceBuffer) {
        glGenBuffers(1, &m_InstanceBuffer);
    }

    uint32_t count = lodsCount();
    uint8_t flag = flags();

    if(m_triangles.size() < count) {
        m_triangles.resize(count);
        m_vertices.resize(count);

        glGenBuffers(count, &m_triangles[0]);
        glGenBuffers(count, &m_vertices[0]);

        if(flag & Mesh::Normals) {
            m_normals.resize(count);
            glGenBuffers(count, &m_normals[0]);
        }
        if(flag & Mesh::Tangents) {
            m_tangents.resize(count);
            glGenBuffers(count, &m_tangents[0]);
        }
        if(flag & Mesh::Uv0) {
            m_uv0.resize(count);
            glGenBuffers(count, &m_uv0[0]);
        }
        if(flag & Mesh::Skinned) {
            m_weights.resize(count);
            glGenBuffers(count, &m_weights[0]);

            m_bones.resize(count);
            glGenBuffers(count, &m_bones[0]);
        }
    }

    bool dynamic = isDynamic();

    for(uint32_t i = 0; i < count; i++) {
        Lod *l = lod(i);

        uint32_t vCount = l->vertices().size();
        if(!l->vertices().empty()) {
            glBindBuffer(GL_ARRAY_BUFFER, m_vertices[i]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3) * vCount, &l->vertices()[0], (dynamic) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
        }
        if(!l->indices().empty()) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_triangles[i]);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * l->indices().size(), &l->indices()[0], (dynamic) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
        }
        if(flag & Mesh::Normals) {
            glBindBuffer(GL_ARRAY_BUFFER, m_normals[i]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3) * vCount, &l->normals()[0], (dynamic) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
        }
        if(flag & Mesh::Tangents) {
            glBindBuffer(GL_ARRAY_BUFFER, m_tangents[i]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3) * vCount, &l->tangents()[0], (dynamic) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
        }
        if(flag & Mesh::Uv0) {
            glBindBuffer(GL_ARRAY_BUFFER, m_uv0[i]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(Vector2) * vCount, &l->uv0()[0], (dynamic) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
        }
        if(flag & Mesh::Skinned) {
            glBindBuffer(GL_ARRAY_BUFFER, m_weights[i]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(Vector4) * vCount, &l->weights()[0], (dynamic) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);

            glBindBuffer(GL_ARRAY_BUFFER, m_bones[i]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(Vector4) * vCount, &l->bones()[0], (dynamic) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
        }
        if(m_Vao.size() <= i) {
            m_Vao.push_back(list<VaoStruct *>());
        }
        if(dynamic) {
            for(auto &it : m_Vao[i]) {
                if(it->buffer == buffer) {
                    it->dirty = true;
                }
            }
        }
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void MeshGL::destroyVao(CommandBufferGL *buffer) {
    for(int32_t l = 0; l < lodsCount(); l++) {
        for(auto &it : m_Vao[l]) {
            if(it->buffer == buffer) {
                glDeleteVertexArrays(1, &(it->vao));
            }
        }
    }
}

void MeshGL::destroyVbo() {
    if(m_vertices.empty() && m_triangles.empty()) {
        return;
    }
    glDeleteBuffers(static_cast<int32_t>(m_vertices.size()), &m_vertices[0]);
    glDeleteBuffers(static_cast<int32_t>(m_triangles.size()), &m_triangles[0]);

    uint8_t flag = flags();

    if(flag & Mesh::Normals) {
        glDeleteBuffers(static_cast<int32_t>(m_normals.size()), &m_normals[0]);
    }

    if(flag & Mesh::Tangents) {
        glDeleteBuffers(static_cast<int32_t>(m_tangents.size()), &m_tangents[0]);
    }

    if(flag & Mesh::Uv0) {
        glDeleteBuffers(static_cast<int32_t>(m_uv0.size()), &m_uv0[0]);
    }

    if(flag & Mesh::Skinned) {
        glDeleteBuffers(static_cast<int32_t>(m_weights.size()), &m_weights[0]);
        glDeleteBuffers(static_cast<int32_t>(m_bones.size()), &m_bones[0]);
    }

    glDeleteBuffers(1, &m_InstanceBuffer);

    m_triangles.clear();
    m_vertices.clear();

    m_normals.clear();
    m_tangents.clear();
    m_uv0.clear();

    m_weights.clear();
    m_bones.clear();
}

uint32_t MeshGL::instance() const {
    return m_InstanceBuffer;
}
