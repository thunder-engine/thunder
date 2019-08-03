#include "resources/ameshgl.h"

#include "agl.h"

#include "commandbuffergl.h"

AMeshGL::AMeshGL() :
        m_InstanceBuffer(0) {
}

void AMeshGL::bindVao(CommandBufferGL *buffer, uint32_t lod) {
    switch(state()) {
        case ToBeUpdated: {
            updateVbo();

            setState(Ready);
        } break;
        case Ready: break;
        case Suspend: {
            destroyVbo();
            destroyVao();

            setState(ToBeDeleted);
            return;
        }
        default: return;
    }

    VaoMap *map = &(m_Vao[lod]);
    auto it = map->find(buffer);
    if(it != map->end() && glIsVertexArray(it->second)) {
        glBindVertexArray(it->second);
        return;
    }
    uint32_t id;
    glGenVertexArrays(1, &id);
    glBindVertexArray(id);

    updateVao(lod);

    (*map)[buffer] = id;
}

void AMeshGL::updateVao(uint32_t lod) {
    // indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_triangles[lod]);
    // vertices
    glBindBuffer(GL_ARRAY_BUFFER, m_vertices[lod]);
    glEnableVertexAttribArray(VERTEX_ATRIB);
    glVertexAttribPointer(VERTEX_ATRIB, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    uint8_t flag = flags();

    if(flag & Mesh::ATTRIBUTE_NORMALS) {
        glBindBuffer(GL_ARRAY_BUFFER, m_normals[lod]);
        glEnableVertexAttribArray(NORMAL_ATRIB);
        glVertexAttribPointer(NORMAL_ATRIB, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    }
    if(flag & Mesh::ATTRIBUTE_TANGENTS) {
        glBindBuffer(GL_ARRAY_BUFFER, m_tangents[lod]);
        glEnableVertexAttribArray(TANGENT_ATRIB);
        glVertexAttribPointer(TANGENT_ATRIB, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    }
    if(flag & Mesh::ATTRIBUTE_UV0) {
        glBindBuffer(GL_ARRAY_BUFFER, m_uv0[lod]);
        glEnableVertexAttribArray(UV0_ATRIB);
        glVertexAttribPointer(UV0_ATRIB, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    }
    if(flag & Mesh::ATTRIBUTE_COLOR) {
        glBindBuffer(GL_ARRAY_BUFFER, m_colors[lod]);
        glEnableVertexAttribArray(COLOR_ATRIB);
        glVertexAttribPointer(COLOR_ATRIB, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_InstanceBuffer);
    for(uint32_t i = 0; i < 4; i++) {
        glEnableVertexAttribArray(INSTANCE_ATRIB + i);
        glVertexAttribPointer(INSTANCE_ATRIB + i, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix4), reinterpret_cast<void *>(i * sizeof(Vector4)));
        glVertexAttribDivisor(INSTANCE_ATRIB + i, 1);
    }
/*
    // uv1
    glEnableVertexAttribArray(2);
    // uv2
    glEnableVertexAttribArray(3);
    // uv3
    glEnableVertexAttribArray(4);
    // colors
    glEnableVertexAttribArray(7);
    // indices
    glEnableVertexAttribArray(8);
    // weights
    glEnableVertexAttribArray(9);
*/
}

void AMeshGL::updateVbo() {
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

        if(flag & Mesh::ATTRIBUTE_NORMALS) {
            m_normals.resize(count);
            glGenBuffers(count, &m_normals[0]);
        }
        if(flag & Mesh::ATTRIBUTE_TANGENTS) {
            m_tangents.resize(count);
            glGenBuffers(count, &m_tangents[0]);
        }
        if(flag & Mesh::ATTRIBUTE_UV0) {
            m_uv0.resize(count);
            glGenBuffers(count, &m_uv0[0]);
        }
    }

    bool dynamic = isDynamic();

    for(uint32_t l = 0; l < count; l++) {
        Lod *lod = getLod(l);

        uint32_t vCount = lod->vertices.size();
        if(!lod->vertices.empty()) {
            glBindBuffer(GL_ARRAY_BUFFER, m_vertices[l]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3) * vCount, &lod->vertices[0], (dynamic) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
        }
        if(!lod->indices.empty()) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_triangles[l]);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * lod->indices.size(), &lod->indices[0], (dynamic) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
        }
        if(flag & Mesh::ATTRIBUTE_NORMALS) {
            glBindBuffer(GL_ARRAY_BUFFER, m_normals[l]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3) * vCount, &lod->normals[0], (dynamic) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
        }
        if(flag & Mesh::ATTRIBUTE_TANGENTS) {
            glBindBuffer(GL_ARRAY_BUFFER, m_tangents[l]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3) * vCount, &lod->tangents[0], (dynamic) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
        }
        if(flag & Mesh::ATTRIBUTE_UV0) {
            glBindBuffer(GL_ARRAY_BUFFER, m_uv0[l]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(Vector2) * vCount, &lod->uv0[0], (dynamic) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
        }
        if(m_Vao.size() <= l) {
            m_Vao.push_back(VaoMap());
        }
        if(dynamic) {
            for(auto it : m_Vao[l]) {
                glDeleteVertexArrays(1, &(it.second));
            }
            m_Vao[l].clear();
        }
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void AMeshGL::destroyVao() {
    for(uint32_t l = 0; l < lodsCount(); l++) {
        for(auto it : m_Vao[l]) {
            glDeleteVertexArrays(1, &(it.second));
        }
    }
    m_Vao.clear();
}

void AMeshGL::destroyVbo() {
    if(m_vertices.empty() && m_triangles.empty()) {
        return;
    }
    glDeleteBuffers(static_cast<int32_t>(m_vertices.size()), &m_vertices[0]);
    glDeleteBuffers(static_cast<int32_t>(m_triangles.size()), &m_triangles[0]);

    uint8_t flag = flags();

    if(flag & Mesh::ATTRIBUTE_NORMALS) {
        glDeleteBuffers(static_cast<int32_t>(m_normals.size()), &m_normals[0]);
    }

    if(flag & Mesh::ATTRIBUTE_TANGENTS) {
        glDeleteBuffers(static_cast<int32_t>(m_tangents.size()), &m_tangents[0]);
    }

    if(flag & Mesh::ATTRIBUTE_UV0) {
        glDeleteBuffers(static_cast<int32_t>(m_uv0.size()), &m_uv0[0]);
    }

    glDeleteBuffers(1, &m_InstanceBuffer);

    m_triangles.clear();
    m_vertices.clear();

    m_normals.clear();
    m_tangents.clear();
    m_uv0.clear();
}

uint32_t AMeshGL::instance() const {
    return m_InstanceBuffer;
}
