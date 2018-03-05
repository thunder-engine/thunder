#include "commandbuffergl.h"

#include "agl.h"
#include "analytics/profiler.h"

#include <resources/amaterialgl.h>
#include <resources/ameshgl.h>
#include <resources/atexturegl.h>

CommandBufferGL::CommandBufferGL() {
    PROFILER_MARKER;

    ICommandBuffer::setHandler(this);

    m_Color     = Vector4();
}

void CommandBufferGL::clearRenderTarget(bool clearColor, const Vector4 &color, bool clearDepth, float depth) {
    PROFILER_MARKER;

    uint32_t flags  = 0;
    if(clearColor) {
        flags   |= GL_COLOR_BUFFER_BIT;
        glClearColor(color.x, color.y, color.z, color.w);
    }
    if(clearDepth) {
        flags   |= GL_DEPTH_BUFFER_BIT;
        glClearDepthf(depth);
    }
    glClear(flags);
}

void CommandBufferGL::drawMesh(const Matrix4 &model, Mesh *mesh, uint32_t surface, uint8_t layer, MaterialInstance *material) {
    PROFILER_MARKER;

    if(mesh && material) {

        m_Model = model;

        AMaterialGL *mat    = dynamic_cast<AMaterialGL *>(material->material());
        uint32_t program    = mat->bind(material, layer, Material::Static);
        if(program) {
            setShaderParams(program);

            AMeshGL *m  = static_cast<AMeshGL *>(mesh);
            if(mesh->isModified()) {
                m->createVbo();
            }
            uint32_t lod    = 0;
            uint32_t id;
            glGenVertexArrays(1, &id);
            glBindVertexArray(id);
            // indices
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->m_triangles[surface][lod]);
            // vertices
            glBindBuffer(GL_ARRAY_BUFFER, m->m_vertices[surface][lod]);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

            uint8_t flags   = mesh->flags();
            glEnableVertexAttribArray(0);
            if(flags & Mesh::ATTRIBUTE_UV0) {
                glBindBuffer(GL_ARRAY_BUFFER, m->m_uv0[surface][lod]);
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
                glEnableVertexAttribArray(1);
            }
            //// uv1
            //glEnableVertexAttribArray(2);
            //// uv2
            //glEnableVertexAttribArray(3);
            //// uv3
            //glEnableVertexAttribArray(4);
            if(flags & Mesh::ATTRIBUTE_NORMALS) {
                glBindBuffer(GL_ARRAY_BUFFER, m->m_normals[surface][lod]);
                glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, 0, 0);
                glEnableVertexAttribArray(5);
            }
            if(flags & Mesh::ATTRIBUTE_TANGENTS) {
                glBindBuffer(GL_ARRAY_BUFFER, m->m_tangents[surface][lod]);
                glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, 0, 0);
                glEnableVertexAttribArray(6);
            }
            //// colors
            //glEnableVertexAttribArray(5);
            //// indices
            //glEnableVertexAttribArray(8);
            //// weights
            //glEnableVertexAttribArray(9);

            Mesh::Modes mode    = mesh->mode(surface);
            if(mode > Mesh::MODE_LINES) {
                uint32_t count  = mesh->vertexCount(surface, lod);
                glDrawArrays((mode == Mesh::MODE_TRIANGLE_STRIP) ? GL_TRIANGLE_STRIP : GL_LINE_STRIP, 0, count);

                PROFILER_STAT(POLYGONS, count - 2);
            } else {
                uint32_t count  = mesh->indexCount(surface, lod);
                glDrawElements((mode == Mesh::MODE_TRIANGLES) ? GL_TRIANGLES : GL_LINES,
                               count, GL_UNSIGNED_INT, 0);

                PROFILER_STAT(POLYGONS, count / 3);
            }
            PROFILER_STAT(DRAWCALLS, 1);
            // vertices
            glDisableVertexAttribArray(0);
            // uv0
            glDisableVertexAttribArray(1);
            // uv1
            glDisableVertexAttribArray(2);
            // uv2
            glDisableVertexAttribArray(3);
            // uv3
            glDisableVertexAttribArray(4);
            // normals
            glDisableVertexAttribArray(5);
            // tangents
            glDisableVertexAttribArray(6);
            // colors
            glDisableVertexAttribArray(7);
            // indices
            glDisableVertexAttribArray(8);
            // weights
            glDisableVertexAttribArray(9);

            glBindVertexArray(0);
            glDeleteVertexArrays(1, &id);

            mat->unbind(layer);
        }
    }
}

void CommandBufferGL::setRenderTarget(uint8_t numberColors, const Texture *colors, const Texture *depth) {
    PROFILER_MARKER;

    for(int i = 0; i < numberColors; i++) {
        glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, static_cast<const ATextureGL *>(&colors[i])->id(), 0 );
    }
    if(depth) {
        glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, static_cast<const ATextureGL *>(depth)->id(), 0 );
    }
}

void CommandBufferGL::setShaderParams(uint32_t program) {
    PROFILER_MARKER;

    int location;
    location    = glGetUniformLocation(program, "transform.color");
    if(location > -1) {
        glUniform4fv(location, 1, m_Color.v);
    }
    location	= glGetUniformLocation(program, "transform.view");
    if(location > -1) {
        glUniformMatrix4fv(location, 1, GL_FALSE, m_View.mat);
    }
    location	= glGetUniformLocation(program, "transform.model");
    if(location > -1) {
        glUniformMatrix4fv(location, 1, GL_FALSE, m_Model.mat);
    }
    location	= glGetUniformLocation(program, "transform.projection");
    if(location > -1) {
        glUniformMatrix4fv(location, 1, GL_FALSE, m_Projection.mat);
    }
    location	= glGetUniformLocation(program, "transform.mv");
    if(location > -1) {
        glUniformMatrix4fv(location, 1, GL_FALSE, modelView().mat);
    }
    location	= glGetUniformLocation(program, "transform.mvpi");
    if(location > -1) {
        glUniformMatrix4fv(location, 1, GL_FALSE, ((m_Projection * modelView()).inverse()).mat);
    }
    // Push uniform values to shader
    for(const auto &it : m_Uniforms) {
        location    = glGetUniformLocation(program, it.first.c_str());
        if(location > -1) {
            const AVariant &data= it.second;
            switch(data.type()) {
                case AMetaType::VECTOR2:    glUniform2fv(location, 1, data.toVector2().v); break;
                case AMetaType::VECTOR3:    glUniform3fv(location, 1, data.toVector3().v); break;
                case AMetaType::VECTOR4:    glUniform4fv(location, 1, data.toVector4().v); break;
                default:                    glUniform1f (location, data.toDouble()); break;
            }
        }
    }
}

void CommandBufferGL::setColor(const Vector4 &color) {
    m_Color = color;
}

void CommandBufferGL::setViewProjection(const Matrix4 &view, const Matrix4 &projection) {
    m_View  = view;
    m_Projection    = projection;
}

void CommandBufferGL::setGlobalValue(const char *name, const AVariant &value) {
    m_Uniforms[name]    = value;
}
