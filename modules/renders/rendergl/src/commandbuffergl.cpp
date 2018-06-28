#include "commandbuffergl.h"

#include "agl.h"
#include "analytics/profiler.h"

#include <resources/amaterialgl.h>
#include <resources/ameshgl.h>
#include <resources/atexturegl.h>

#include "resources/arendertexturegl.h"

#define TRANSFORM_BIND 0

#define COLOR_BIND 2
#define TIMER_BIND 3

const char *gVertex = ".embedded/BasePass.vert";

#define TYPE_STATIC "#define TYPE_STATIC 1"
#define TYPE_SKINNED "#define TYPE_SKINNED 1"
#define TYPE_BILLBOARD "#define TYPE_BILLBOARD 1"
#define TYPE_AXISALIGNED "#define TYPE_AXISALIGNED 1"

CommandBufferGL::CommandBufferGL() {
    PROFILER_MARKER;
/*
    glGenBuffers(1, &m_Transform);
    glBindBuffer(GL_UNIFORM_BUFFER, m_Transform);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(TransformData), &m_TransformData, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferBase(GL_UNIFORM_BUFFER, TRANSFORM_BIND, m_Transform);
*/
    m_StaticVertex.clear();
    m_Static    = m_StaticVertex.buildShader(AMaterialGL::Vertex, gVertex, TYPE_STATIC);

    m_ModelLocation = glGetUniformLocation(m_Static, "t_model");

    glGenProgramPipelinesEXT(1, &m_Pipeline);
    glUseProgramStagesEXT(m_Pipeline, GL_VERTEX_SHADER_BIT_EXT, m_Static);
}

CommandBufferGL::~CommandBufferGL() {
    glDeleteProgramPipelinesEXT(1, &m_Pipeline);

    glDeleteProgram(m_Static);
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
        AMeshGL *m      = static_cast<AMeshGL *>(mesh);
        uint32_t lod    = 0;
        uint32_t id     = handle(m, surface, lod);

        AMaterialGL *mat    = dynamic_cast<AMaterialGL *>(material->material());
        uint32_t program    = mat->bind(*this, material, layer);
        if(program) {
            glProgramUniformMatrix4fvEXT(m_Static, m_ModelLocation, 1, GL_FALSE, model.mat);

            glProgramUniform1fEXT   (program, TIMER_BIND, Timer::time());
            glProgramUniform4fvEXT  (program, COLOR_BIND, 1, m_Color.v);

            // Push uniform values to shader
            int32_t location;
            for(const auto &it : m_Uniforms) {
                location    = glGetUniformLocation(program, it.first.c_str());
                if(location > -1) {
                    const Variant &data= it.second;
                    switch(data.type()) {
                        case MetaType::VECTOR2: glProgramUniform2fvEXT      (program, location, 1, data.toVector2().v); break;
                        case MetaType::VECTOR3: glProgramUniform3fvEXT      (program, location, 1, data.toVector3().v); break;
                        case MetaType::VECTOR4: glProgramUniform4fvEXT      (program, location, 1, data.toVector4().v); break;
                        case MetaType::MATRIX4: glProgramUniformMatrix4fvEXT(program, location, 1, GL_FALSE, data.toMatrix4().mat); break;
                        default:                glProgramUniform1fEXT       (program, location, data.toFloat()); break;
                    }
                }
            }

            for(const auto &it : material->params()) {
                location    = glGetUniformLocation(program, it.first.c_str());
                if(location > -1) {
                    const MaterialInstance::Info &data  = it.second;
                    switch(data.type) {
                        case 0: break;
                        case MetaType::INTEGER: glProgramUniform1ivEXT      (program, location, data.count, static_cast<const int32_t *>(data.ptr)); break;
                        case MetaType::VECTOR2: glProgramUniform2fvEXT      (program, location, data.count, static_cast<const float *>(data.ptr)); break;
                        case MetaType::VECTOR3: glProgramUniform3fvEXT      (program, location, data.count, static_cast<const float *>(data.ptr)); break;
                        case MetaType::VECTOR4: glProgramUniform4fvEXT      (program, location, data.count, static_cast<const float *>(data.ptr)); break;
                        case MetaType::MATRIX4: glProgramUniformMatrix4fvEXT(program, location, data.count, GL_FALSE, static_cast<const float *>(data.ptr)); break;
                        default:                glProgramUniform1fvEXT      (program, location, data.count, static_cast<const float *>(data.ptr)); break;
                    }
                }
            }

            glEnable(GL_TEXTURE_2D);
            uint8_t i   = 0;
            for(auto it : mat->textures()) {
                const Texture *tex  = static_cast<const ATextureGL *>(it.second);
                const Texture *tmp  = material->texture(it.first.c_str());
                if(tmp) {
                    tex = tmp;
                } else {
                    tmp = texture(it.first.c_str());
                    if(tmp) {
                        tex = tmp;
                    }
                }

                if(tex) {
                    glActiveTexture(GL_TEXTURE0 + i);
                    glBindTexture((tex->isCubemap()) ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, (uint32_t)(size_t)tex->nativeHandle());
                }
                i++;
            }

            glUseProgramStagesEXT(m_Pipeline, GL_FRAGMENT_SHADER_BIT_EXT, program);
            glBindProgramPipelineEXT(m_Pipeline);

            glBindVertexArray(id);

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

            glBindVertexArray(0);

            mat->unbind(layer);

            glBindProgramPipelineEXT(0);
        }
    }
}

void CommandBufferGL::setRenderTarget(const TargetBuffer &target, const RenderTexture *depth) {
    PROFILER_MARKER;

    uint32_t colors[8];

    uint32_t buffer = 0;
    if(!target.empty()) {
        for(int i = 0; i < target.size(); i++) {
            const ARenderTextureGL *c = static_cast<const ARenderTextureGL *>(target[i]);
            if(i == 0) {
                buffer  = c->buffer();
                glBindFramebuffer(GL_FRAMEBUFFER, buffer);
            }
            colors[i]   = GL_COLOR_ATTACHMENT0 + i;
            glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, (uint32_t)(size_t)c->nativeHandle(), 0 );
        }
    }

    if(depth) {
        const ARenderTextureGL *t = static_cast<const ARenderTextureGL *>(depth);
        if(!buffer) {
            glBindFramebuffer(GL_FRAMEBUFFER, t->buffer());
        }
        glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, (uint32_t)(size_t)t->nativeHandle(), 0 );
    }

    if(target.size() > 1) {
        glDrawBuffers( target.size(), colors );
    }
}

const Texture *CommandBufferGL::texture(const char *name) const {
    auto it = m_Textures.find(name);
    if(it != m_Textures.end()) {
        return (*it).second;
    }
    return nullptr;
}

uint32_t CommandBufferGL::handle(AMeshGL *mesh, uint32_t surface, uint32_t lod) {
    uint32_t id = 0;

    uint32_t key    = mesh->m_triangles[surface][lod];
    auto it = m_Objects.find(key);
    if(it == m_Objects.end()) {
        glGenVertexArrays(1, &id);
        glBindVertexArray(id);

        // indices
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->m_triangles[surface][lod]);
        // vertices
        glBindBuffer(GL_ARRAY_BUFFER, mesh->m_vertices[surface][lod]);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        uint8_t flags   = mesh->flags();
        glEnableVertexAttribArray(0);
        if(flags & Mesh::ATTRIBUTE_UV0) {
            glBindBuffer(GL_ARRAY_BUFFER, mesh->m_uv0[surface][lod]);
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
            glBindBuffer(GL_ARRAY_BUFFER, mesh->m_normals[surface][lod]);
            glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(5);
        }
        if(flags & Mesh::ATTRIBUTE_TANGENTS) {
            glBindBuffer(GL_ARRAY_BUFFER, mesh->m_tangents[surface][lod]);
            glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(6);
        }
        //// colors
        //glEnableVertexAttribArray(7);
        //// indices
        //glEnableVertexAttribArray(8);
        //// weights
        //glEnableVertexAttribArray(9);

        glBindVertexArray(0);

        m_Objects[key]  = id;
    } else {
        id  = it->second;
    }

    return id;
}

void CommandBufferGL::updateValues() {
    //glBindBuffer(GL_UNIFORM_BUFFER, m_Transform);
    //void* p = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
    //memcpy(p, &m_TransformData, sizeof(m_TransformData));
    //glUnmapBuffer(GL_UNIFORM_BUFFER);
}

void CommandBufferGL::setColor(const Vector4 &color) {
    m_Color = color;
    //updateValues();
}

void CommandBufferGL::setViewProjection(const Matrix4 &view, const Matrix4 &projection) {
    m_View          = view;
    m_Projection    = projection;

    int32_t location	= glGetUniformLocation(m_Static, "t_view");
    if(location > -1) {
        glProgramUniformMatrix4fvEXT(m_Static, location, 1, GL_FALSE, m_View.mat);
    }
    location	= glGetUniformLocation(m_Static, "t_projection");
    if(location > -1) {
        glProgramUniformMatrix4fvEXT(m_Static, location, 1, GL_FALSE, m_Projection.mat);
    }

    //updateValues();
}

void CommandBufferGL::setGlobalValue(const char *name, const Variant &value) {
    m_Uniforms[name]    = value;
}

void CommandBufferGL::setGlobalTexture(const char *name, const Texture *value) {
    m_Textures[name]    = value;
}

void CommandBufferGL::setViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    glViewport(x, y, width, height);
}
