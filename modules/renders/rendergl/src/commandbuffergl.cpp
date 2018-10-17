#include "commandbuffergl.h"

#include "agl.h"
#include "analytics/profiler.h"

#include <resources/amaterialgl.h>
#include <resources/ameshgl.h>
#include <resources/atexturegl.h>

#include "resources/arendertexturegl.h"

#define TRANSFORM_BIND 0

#define COLOR_BIND  2
#define TIMER_BIND  3
#define CLIP_BIND   4

#define INSTANCE_BIND   4

const char *gVertex = ".embedded/BasePass.vert";

#define TYPE_STATIC "#define TYPE_STATIC 1"
#define TYPE_SKINNED "#define TYPE_SKINNED 1"
#define TYPE_BILLBOARD "#define TYPE_BILLBOARD 1"
#define TYPE_AXISALIGNED "#define TYPE_AXISALIGNED 1"

#define INSTACED "#define INSTANCING 1"

CommandBufferGL::CommandBufferGL() {
    PROFILER_MARKER;

    m_StaticVertex.clear();
    string flags    = TYPE_STATIC;
    m_Static    = m_StaticVertex.buildShader(AMaterialGL::Vertex, gVertex, flags);

    flags   += "\n";
    flags   += INSTACED;
    m_Instanced = m_StaticVertex.buildShader(AMaterialGL::Vertex, gVertex, flags);

    m_ModelLocation = glGetUniformLocation(m_Static, "t_model");

    glGenBuffers(1, &m_InstanceBuffer);

#if GL_ES_VERSION_2_0
    glGenProgramPipelinesEXT(1, &m_Pipeline);
#else
    glGenProgramPipelines(1, &m_Pipeline);
#endif
}

CommandBufferGL::~CommandBufferGL() {
#if GL_ES_VERSION_2_0
    glDeleteProgramPipelinesEXT(1, &m_Pipeline);
#else
    glDeleteProgramPipelines(1, &m_Pipeline);
#endif
    glDeleteBuffers(1, &m_InstanceBuffer);

    glDeleteProgram(m_Static);

    glDeleteProgram(m_Instanced);
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

void CommandBufferGL::putUniforms(uint32_t fragment, MaterialInstance *instance) {
    glProgramUniform1fEXT   (fragment, TIMER_BIND, Timer::time());
    glProgramUniform1fEXT   (fragment, CLIP_BIND,  0.99f);
    glProgramUniform4fvEXT  (fragment, COLOR_BIND, 1, m_Color.v);

    int32_t location;
    // Push uniform values to shader
    for(const auto &it : m_Uniforms) {
        location    = glGetUniformLocation(fragment, it.first.c_str());
        if(location > -1) {
            const Variant &data= it.second;
            switch(data.type()) {
                case MetaType::VECTOR2: glProgramUniform2fvEXT      (fragment, location, 1, data.toVector2().v); break;
                case MetaType::VECTOR3: glProgramUniform3fvEXT      (fragment, location, 1, data.toVector3().v); break;
                case MetaType::VECTOR4: glProgramUniform4fvEXT      (fragment, location, 1, data.toVector4().v); break;
                case MetaType::MATRIX4: glProgramUniformMatrix4fvEXT(fragment, location, 1, GL_FALSE, data.toMatrix4().mat); break;
                default:                glProgramUniform1fEXT       (fragment, location, data.toFloat()); break;
            }
        }
    }

    for(const auto &it : instance->params()) {
        location    = glGetUniformLocation(fragment, it.first.c_str());
        if(location > -1) {
            const MaterialInstance::Info &data  = it.second;
            switch(data.type) {
                case 0: break;
                case MetaType::INTEGER: glProgramUniform1ivEXT      (fragment, location, data.count, static_cast<const int32_t *>(data.ptr)); break;
                case MetaType::VECTOR2: glProgramUniform2fvEXT      (fragment, location, data.count, static_cast<const float *>(data.ptr)); break;
                case MetaType::VECTOR3: glProgramUniform3fvEXT      (fragment, location, data.count, static_cast<const float *>(data.ptr)); break;
                case MetaType::VECTOR4: glProgramUniform4fvEXT      (fragment, location, data.count, static_cast<const float *>(data.ptr)); break;
                case MetaType::MATRIX4: glProgramUniformMatrix4fvEXT(fragment, location, data.count, GL_FALSE, static_cast<const float *>(data.ptr)); break;
                default:                glProgramUniform1fvEXT      (fragment, location, data.count, static_cast<const float *>(data.ptr)); break;
            }
        }
    }

    AMaterialGL *mat    = static_cast<AMaterialGL *>(instance->material());

    glEnable(GL_TEXTURE_2D);
    uint8_t i   = 0;
    for(auto it : mat->textures()) {
        const Texture *tex  = static_cast<const ATextureGL *>(it.second);
        const Texture *tmp  = instance->texture(it.first.c_str());
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
}

void CommandBufferGL::drawMesh(const Matrix4 &model, Mesh *mesh, uint32_t surface, uint8_t layer, MaterialInstance *material) {
    PROFILER_MARKER;

    if(mesh && material) {
        AMeshGL *m      = static_cast<AMeshGL *>(mesh);
        uint32_t lod    = 0;

        AMaterialGL *mat    = static_cast<AMaterialGL *>(material->material());
        uint32_t program    = mat->bind(layer);
        if(program) {
            glProgramUniformMatrix4fvEXT(m_Static, m_ModelLocation, 1, GL_FALSE, model.mat);

            putUniforms(program, material);
#if GL_ES_VERSION_2_0
            glUseProgramStagesEXT(m_Pipeline, GL_VERTEX_SHADER_BIT_EXT, m_Static);
            glUseProgramStagesEXT(m_Pipeline, GL_FRAGMENT_SHADER_BIT_EXT, program);
            glBindProgramPipelineEXT(m_Pipeline);
#else
            glUseProgramStages(m_Pipeline, GL_VERTEX_SHADER_BIT, m_Static);
            glUseProgramStages(m_Pipeline, GL_FRAGMENT_SHADER_BIT, program);
            glBindProgramPipeline(m_Pipeline);
#endif
            bindVao(m, surface, lod);

            Mesh::Modes mode    = mesh->mode(surface);
            if(mode > Mesh::MODE_LINES) {
                uint32_t vert   = mesh->vertexCount(surface, lod);
                glDrawArrays((mode == Mesh::MODE_TRIANGLE_STRIP) ? GL_TRIANGLE_STRIP : GL_LINE_STRIP, 0, vert);

                PROFILER_STAT(POLYGONS, vert - 2);
            } else {
                uint32_t index  = mesh->indexCount(surface, lod);
                glDrawElements((mode == Mesh::MODE_TRIANGLES) ? GL_TRIANGLES : GL_LINES,
                               index, GL_UNSIGNED_INT, 0);

                PROFILER_STAT(POLYGONS, index / 3);
            }
            PROFILER_STAT(DRAWCALLS, 1);

            glBindVertexArray(0);

            mat->unbind(layer);
#if GL_ES_VERSION_2_0
            glBindProgramPipelineEXT(0);
#else
            glBindProgramPipeline(0);
#endif
        }
    }
}

void CommandBufferGL::drawMeshInstanced(const Matrix4 *models, uint32_t count, Mesh *mesh, uint32_t surface, uint8_t layer, MaterialInstance *material) {
    PROFILER_MARKER;

    if(mesh && material) {
        AMeshGL *m      = static_cast<AMeshGL *>(mesh);
        uint32_t lod    = 0;

        AMaterialGL *mat    = static_cast<AMaterialGL *>(material->material());
        uint32_t program    = mat->bind(layer);
        if(program) {
            glBindBuffer(GL_ARRAY_BUFFER, m_InstanceBuffer);
            glBufferData(GL_ARRAY_BUFFER, count * sizeof(Matrix4), models, GL_DYNAMIC_DRAW);

            putUniforms(program, material);
#if GL_ES_VERSION_2_0
            glUseProgramStagesEXT(m_Pipeline, GL_VERTEX_SHADER_BIT_EXT, m_Instanced);
            glUseProgramStagesEXT(m_Pipeline, GL_FRAGMENT_SHADER_BIT_EXT, program);
            glBindProgramPipelineEXT(m_Pipeline);
#else
            glUseProgramStages(m_Pipeline, GL_VERTEX_SHADER_BIT, m_Instanced);
            glUseProgramStages(m_Pipeline, GL_FRAGMENT_SHADER_BIT, program);
            glBindProgramPipeline(m_Pipeline);
#endif
            bindVao(m, surface, lod, m_InstanceBuffer);

            Mesh::Modes mode    = mesh->mode(surface);
            if(mode > Mesh::MODE_LINES) {
                uint32_t vert   = mesh->vertexCount(surface, lod);
                glDrawArraysInstanced((mode == Mesh::MODE_TRIANGLE_STRIP) ? GL_TRIANGLE_STRIP : GL_LINE_STRIP, 0, vert, count);

                PROFILER_STAT(POLYGONS, index - 2 * count);
            } else {
                uint32_t index  = mesh->indexCount(surface, lod);
                glDrawElementsInstanced((mode == Mesh::MODE_TRIANGLES) ? GL_TRIANGLES : GL_LINES,
                               index, GL_UNSIGNED_INT, 0, count);

                PROFILER_STAT(POLYGONS, (index / 3) * count);
            }
            PROFILER_STAT(DRAWCALLS, 1);

            glBindVertexArray(0);

            mat->unbind(layer);
#if GL_ES_VERSION_2_0
            glBindProgramPipelineEXT(0);
#else
            glBindProgramPipeline(0);
#endif
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

void CommandBufferGL::setRenderTarget(uint32_t target) {
    glBindFramebuffer(GL_FRAMEBUFFER, target);
}

const Texture *CommandBufferGL::texture(const char *name) const {
    auto it = m_Textures.find(name);
    if(it != m_Textures.end()) {
        return (*it).second;
    }
    return nullptr;
}

void CommandBufferGL::notify(uint32_t index) {
    auto it = m_Objects.find(index);
    if(it != m_Objects.end()) {
        m_Objects.erase(it);
    }
}

void CommandBufferGL::bindVao(AMeshGL *mesh, uint32_t surface, uint32_t lod, uint32_t instance) {
    uint32_t key    = mesh->m_triangles[surface][lod];
    auto it = m_Objects.find(key);
    if(it == m_Objects.end()) {
        mesh->subscribe(this);

        uint32_t id;
        glGenVertexArrays(1, &id);
        glBindVertexArray(id);

        // indices
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->m_triangles[surface][lod]);
        // vertices
        glBindBuffer(GL_ARRAY_BUFFER, mesh->m_vertices[surface][lod]);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        uint8_t flags   = mesh->flags();
        if(flags & Mesh::ATTRIBUTE_NORMALS) {
            glBindBuffer(GL_ARRAY_BUFFER, mesh->m_normals[surface][lod]);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
        }
        if(flags & Mesh::ATTRIBUTE_TANGENTS) {
            glBindBuffer(GL_ARRAY_BUFFER, mesh->m_tangents[surface][lod]);
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
        }
        if(flags & Mesh::ATTRIBUTE_UV0) {
            glBindBuffer(GL_ARRAY_BUFFER, mesh->m_uv0[surface][lod]);
            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);
        }

        if(instance) {
            glBindBuffer(GL_ARRAY_BUFFER, instance);
            for(int i = 0; i < 4; i++) {
                glEnableVertexAttribArray(INSTANCE_BIND + i);
                glVertexAttribPointer(INSTANCE_BIND + i, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix4), (void *)(i * sizeof(Vector4)));
                glVertexAttribDivisor(INSTANCE_BIND + i, 1);
            }
        } else {
            //// uv1
            //glEnableVertexAttribArray(2);
            //// uv2
            //glEnableVertexAttribArray(3);
            //// uv3
            //glEnableVertexAttribArray(4);
            //// colors
            //glEnableVertexAttribArray(7);
            //// indices
            //glEnableVertexAttribArray(8);
            //// weights
            //glEnableVertexAttribArray(9);
        }
        m_Objects[key]  = id;
    } else {
        glBindVertexArray(it->second);
    }
}

void CommandBufferGL::setColor(const Vector4 &color) {
    m_Color = color;
}

void CommandBufferGL::setViewProjection(const Matrix4 &view, const Matrix4 &projection) {
    m_View          = view;
    m_Projection    = projection;

    int32_t location;
    location    = glGetUniformLocation(m_Static, "t_view");
    if(location > -1) {
        glProgramUniformMatrix4fvEXT(m_Static, location, 1, GL_FALSE, m_View.mat);
    }
    location	= glGetUniformLocation(m_Static, "t_projection");
    if(location > -1) {
        glProgramUniformMatrix4fvEXT(m_Static, location, 1, GL_FALSE, m_Projection.mat);
    }

    location    = glGetUniformLocation(m_Instanced, "t_view");
    if(location > -1) {
        glProgramUniformMatrix4fvEXT(m_Instanced, location, 1, GL_FALSE, m_View.mat);
    }
    location	= glGetUniformLocation(m_Instanced, "t_projection");
    if(location > -1) {
        glProgramUniformMatrix4fvEXT(m_Instanced, location, 1, GL_FALSE, m_Projection.mat);
    }
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
