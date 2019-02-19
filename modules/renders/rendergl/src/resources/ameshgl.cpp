#include "resources/ameshgl.h"

#include "agl.h"

#include "commandbuffergl.h"

AMeshGL::AMeshGL() :
        m_InstanceBuffer(0) {
}

AMeshGL::~AMeshGL() {
    clear();
}

void AMeshGL::apply() {
    Mesh::apply();

    if(!m_InstanceBuffer) {
        glGenBuffers(1, &m_InstanceBuffer);
    }

    for(uint32_t s = 0; s < m_Surfaces.size(); s++) {
        uint8_t lods = m_Surfaces[s].lods.size();

        if(m_vertices.size() <= s) {
            IndexVector tris(lods);
            glGenBuffers(lods, &tris[0]);
            m_triangles.push_back(tris);

            IndexVector vert(lods);
            glGenBuffers(lods, &vert[0]);
            m_vertices.push_back(vert);

            if(m_Flags & Mesh::ATTRIBUTE_NORMALS) {
                IndexVector norm(lods);
                glGenBuffers(lods, &norm[0]);
                m_normals.push_back(norm);
            }
            if(m_Flags & Mesh::ATTRIBUTE_TANGENTS) {
                IndexVector tang(lods);
                glGenBuffers(lods, &tang[0]);
                m_tangents.push_back(tang);
            }
            if(m_Flags & Mesh::ATTRIBUTE_UV0) {
                IndexVector uv  (lods);
                glGenBuffers(lods, &uv[0]);
                m_uv0.push_back(uv);
            }
            m_Vao.push_back(vector<VaoMap>());
        }

        for(uint32_t l = 0; l < lods; l++) {
            Lod *lod = &m_Surfaces[s].lods[l];
            uint32_t vCount = lod->vertices.size();

            if(!lod->vertices.empty()) {
                glBindBuffer(GL_ARRAY_BUFFER, m_vertices[s][l]);
                glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3) * vCount, &lod->vertices[0], (m_Dynamic) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
            }
            if(!lod->indices.empty()) {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_triangles[s][l]);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * lod->indices.size(), &lod->indices[0], (m_Dynamic) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
            }
            if(m_Flags & Mesh::ATTRIBUTE_NORMALS) {
                glBindBuffer(GL_ARRAY_BUFFER, m_normals[s][l]);
                glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3) * vCount, &lod->normals[0], (m_Dynamic) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
            }
            if(m_Flags & Mesh::ATTRIBUTE_TANGENTS) {
                glBindBuffer(GL_ARRAY_BUFFER, m_tangents[s][l]);
                glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3) * vCount, &lod->tangents[0], (m_Dynamic) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
            }
            if(m_Flags & Mesh::ATTRIBUTE_UV0) {
                glBindBuffer(GL_ARRAY_BUFFER, m_uv0[s][l]);
                glBufferData(GL_ARRAY_BUFFER, sizeof(Vector2) * vCount, &lod->uv0[0], (m_Dynamic) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
            }
            if(m_Vao[s].size() <= l) {
                m_Vao[s].push_back(VaoMap());
            }
            if(m_Dynamic) {
                for(auto it : m_Vao[s][l]) {
                    glDeleteVertexArrays(1, &(it.second));
                }
                m_Vao[s][l].clear();
            }
        }
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void AMeshGL::bindVao(CommandBufferGL *buffer, uint32_t surface, uint32_t lod) {
    VaoMap *map = &(m_Vao[surface][lod]);
    auto it = map->find(buffer);
    if(it != map->end() && glIsVertexArray(it->second)) {
        glBindVertexArray(it->second);
        return;
    }
    uint32_t id;
    glGenVertexArrays(1, &id);
    glBindVertexArray(id);

    updateVao(surface, lod);

    (*map)[buffer] = id;
}

void AMeshGL::updateVao(uint32_t surface, uint32_t lod) {
    // indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_triangles[surface][lod]);
    // vertices
    glBindBuffer(GL_ARRAY_BUFFER, m_vertices[surface][lod]);
    glEnableVertexAttribArray(VERTEX_ATRIB);
    glVertexAttribPointer(VERTEX_ATRIB, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    if(m_Flags & Mesh::ATTRIBUTE_NORMALS) {
        glBindBuffer(GL_ARRAY_BUFFER, m_normals[surface][lod]);
        glEnableVertexAttribArray(NORMAL_ATRIB);
        glVertexAttribPointer(NORMAL_ATRIB, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    }
    if(m_Flags & Mesh::ATTRIBUTE_TANGENTS) {
        glBindBuffer(GL_ARRAY_BUFFER, m_tangents[surface][lod]);
        glEnableVertexAttribArray(TANGENT_ATRIB);
        glVertexAttribPointer(TANGENT_ATRIB, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    }
    if(m_Flags & Mesh::ATTRIBUTE_UV0) {
        glBindBuffer(GL_ARRAY_BUFFER, m_uv0[surface][lod]);
        glEnableVertexAttribArray(UV0_ATRIB);
        glVertexAttribPointer(UV0_ATRIB, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    }
    if(m_Flags & Mesh::ATTRIBUTE_COLOR) {
        glBindBuffer(GL_ARRAY_BUFFER, m_colors[surface][lod]);
        glEnableVertexAttribArray(COLOR_ATRIB);
        glVertexAttribPointer(COLOR_ATRIB, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_InstanceBuffer);
    for(uint32_t i = 0; i < 4; i++) {
        glEnableVertexAttribArray(INSTANCE_ATRIB + i);
        glVertexAttribPointer(INSTANCE_ATRIB + i, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix4), (void *)(i * sizeof(Vector4)));
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

uint32_t AMeshGL::instance() const {
    return m_InstanceBuffer;
}

void AMeshGL::clear() {
    if(m_vertices.empty() && m_triangles.empty()) {
        Mesh::clear();
        return;
    }
    for(uint32_t s = 0; s < m_Surfaces.size(); s++) {
        {
            uint32_t size   = m_vertices[s].size();
            glDeleteBuffers(size, &m_vertices[s][0]);
        }
        {
            uint32_t size   = m_triangles[s].size();
            glDeleteBuffers(size, &m_triangles[s][0]);
        }

        {
            if(m_Flags & Mesh::ATTRIBUTE_NORMALS) {
                uint32_t size   = m_normals[s].size();
                glDeleteBuffers(size, &m_normals[s][0]);
            }
        }
        {
            if(m_Flags & Mesh::ATTRIBUTE_TANGENTS) {
                uint32_t size   = m_tangents[s].size();
                glDeleteBuffers(size, &m_tangents[s][0]);
            }
        }
        {
            if(m_Flags & Mesh::ATTRIBUTE_UV0) {
                uint32_t size   = m_uv0[s].size();
                glDeleteBuffers(size, &m_uv0[s][0]);
            }
        }
        uint8_t lods = m_Surfaces[s].lods.size();
        for(uint32_t l = 0; l < lods; l++) {
            if(m_Vao.size() > s && m_Vao[s].size() > l) {
                for(auto it : m_Vao[s][l]) {
                    glDeleteVertexArrays(1, &(it.second));
                }
            }
        }
        m_Vao.clear();
    }

    glDeleteBuffers(1, &m_InstanceBuffer);

    m_triangles.clear();
    m_vertices.clear();

    m_normals.clear();
    m_tangents.clear();
    m_uv0.clear();

    Mesh::clear();
}
