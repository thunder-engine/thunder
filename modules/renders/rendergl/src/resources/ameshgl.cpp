#include "resources/ameshgl.h"

#include "agl.h"

#include "commandbuffergl.h"

AMeshGL::AMeshGL() :
        Mesh() {
}

AMeshGL::~AMeshGL() {
    clear();
}

void AMeshGL::apply() {
    Mesh::apply();

    for(uint32_t s = 0; s < m_Surfaces.size(); s++) {
        uint8_t lods    = m_Surfaces[s].lods.size();

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
        }

        for(uint32_t l = 0; l < lods; l++) {
            Lod *lod    = &m_Surfaces[s].lods[l];
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
        }
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
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
    }

    m_triangles.clear();
    m_vertices.clear();

    m_normals.clear();
    m_tangents.clear();
    m_uv0.clear();

    Mesh::clear();
}
