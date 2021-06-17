#include "commandbuffergl.h"

#include "agl.h"

#include "resources/materialgl.h"
#include "resources/meshgl.h"
#include "resources/texturegl.h"
#include "resources/rendertexturegl.h"

#include <log.h>
#include <timer.h>

#define MODEL_UNIFORM   0
#define VIEW_UNIFORM    1
#define PROJ_UNIFORM    2

#define COLOR_BIND  3
#define CLIP_BIND   4
#define TIMER_BIND  5

CommandBufferGL::CommandBufferGL() {
    PROFILE_FUNCTION();

}

CommandBufferGL::~CommandBufferGL() {

}

void CommandBufferGL::clearRenderTarget(bool clearColor, const Vector4 &color, bool clearDepth, float depth) {
    PROFILE_FUNCTION();

    uint32_t flags = 0;
    if(clearColor) {
        flags   |= GL_COLOR_BUFFER_BIT;
        glClearColor(color.x, color.y, color.z, color.w);
    }
    if(clearDepth) {
        glDepthMask(GL_TRUE);
        flags   |= GL_DEPTH_BUFFER_BIT;
        glClearDepthf(depth);
    }
    glClear(flags);
}

void CommandBufferGL::putUniforms(uint32_t program, MaterialInstance *instance) {
    int32_t location;

    glUniformMatrix4fv(VIEW_UNIFORM, 1, GL_FALSE, m_View.mat);
    glUniformMatrix4fv(PROJ_UNIFORM, 1, GL_FALSE, m_Projection.mat);

    glUniform1f   (TIMER_BIND, Timer::time());
    glUniform1f   (CLIP_BIND,  0.99f);
    glUniform4fv  (COLOR_BIND, 1, m_Color.v);

    // Push uniform values to shader
    for(const auto &it : m_Uniforms) {
        location = glGetUniformLocation(program, it.first.c_str());
        if(location > -1) {
            const Variant &data = it.second;
            switch(data.type()) {
                case MetaType::VECTOR2: glUniform2fv      (location, 1, data.toVector2().v); break;
                case MetaType::VECTOR3: glUniform3fv      (location, 1, data.toVector3().v); break;
                case MetaType::VECTOR4: glUniform4fv      (location, 1, data.toVector4().v); break;
                case MetaType::MATRIX4: glUniformMatrix4fv(location, 1, GL_FALSE, data.toMatrix4().mat); break;
                default:                glUniform1f       (location, data.toFloat()); break;
            }
        }
    }

    for(const auto &it : instance->params()) {
        location = glGetUniformLocation(program, it.first.c_str());
        if(location > -1) {
            const MaterialInstance::Info &data = it.second;
            switch(data.type) {
                case MetaType::INTEGER: glUniform1iv      (location, data.count, static_cast<const int32_t *>(data.ptr)); break;
                case MetaType::FLOAT:   glUniform1fv      (location, data.count, static_cast<const float *>(data.ptr)); break;
                case MetaType::VECTOR2: glUniform2fv      (location, data.count, static_cast<const float *>(data.ptr)); break;
                case MetaType::VECTOR3: glUniform3fv      (location, data.count, static_cast<const float *>(data.ptr)); break;
                case MetaType::VECTOR4: glUniform4fv      (location, data.count, static_cast<const float *>(data.ptr)); break;
                case MetaType::MATRIX4: glUniformMatrix4fv(location, data.count, GL_FALSE, static_cast<const float *>(data.ptr)); break;
                default: break;
            }
        }
    }

    MaterialGL *mat = static_cast<MaterialGL *>(instance->material());

    uint8_t i = 0;
    for(auto &it : mat->textures()) {
        Texture *tex = static_cast<TextureGL *>(it.second);
        Texture *tmp = instance->texture(it.first.c_str());
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
            uint32_t target = GL_TEXTURE_2D;
            if(tex->isCubemap()) {
                target = GL_TEXTURE_CUBE_MAP;
            }
            glBindTexture(target, (uint32_t)(size_t)tex->nativeHandle());
        }
        i++;
    }
}

void CommandBufferGL::drawMesh(const Matrix4 &model, Mesh *mesh, uint32_t layer, MaterialInstance *material) {
    PROFILE_FUNCTION();

    if(mesh && material) {
        MeshGL *m = static_cast<MeshGL *>(mesh);
        uint32_t lod = 0;
        Lod *l = mesh->lod(lod);
        if(l == nullptr) {
            return;
        }

        MaterialGL *mat = static_cast<MaterialGL *>(material->material());
        uint32_t program = mat->bind(layer, material->surfaceType());
        if(program) {
            glUseProgram(program);

            glUniformMatrix4fv(MODEL_UNIFORM, 1, GL_FALSE, model.mat);

            putUniforms(program, material);

            m->bindVao(this, lod);

            Mesh::TriangleModes mode = static_cast<Mesh::TriangleModes>(mesh->mode());
            if(mode > Mesh::Lines) {
                uint32_t vert = l->vertices().size();
                int32_t glMode = GL_TRIANGLE_STRIP;
                switch(mode) {
                case Mesh::LineStrip:   glMode = GL_LINE_STRIP; break;
                case Mesh::TriangleFan: glMode = GL_TRIANGLE_FAN; break;
                default: break;
                }
                glDrawArrays(glMode, 0, vert);
                PROFILER_STAT(POLYGONS, vert - 2);
            } else {
                uint32_t index = l->indices().size();
                glDrawElements((mode == Mesh::Triangles) ? GL_TRIANGLES : GL_LINES, index, GL_UNSIGNED_INT, nullptr);
                PROFILER_STAT(POLYGONS, index / 3);
            }
            PROFILER_STAT(DRAWCALLS, 1);

            glBindVertexArray(0);
        }
    }
}

void CommandBufferGL::drawMeshInstanced(const Matrix4 *models, uint32_t count, Mesh *mesh, uint32_t layer, MaterialInstance *material) {
    PROFILE_FUNCTION();

    if(mesh && material) {
        MeshGL *m = static_cast<MeshGL *>(mesh);
        uint32_t lod = 0;
        Lod *l = mesh->lod(lod);
        if(l == nullptr) {
            return;
        }

        MaterialGL *mat = static_cast<MaterialGL *>(material->material());
        uint32_t program = mat->bind(layer, material->surfaceType());

        if(program) {
            glUseProgram(program);

            glUniformMatrix4fv(MODEL_UNIFORM, 1, GL_FALSE, Matrix4().mat);

            glBindBuffer(GL_ARRAY_BUFFER, m->instance());
            glBufferData(GL_ARRAY_BUFFER, count * sizeof(Matrix4), models, GL_DYNAMIC_DRAW);

            putUniforms(program, material);

            m->bindVao(this, lod);

            Mesh::TriangleModes mode = static_cast<Mesh::TriangleModes>(mesh->mode());
            if(mode > Mesh::Lines) {
                uint32_t vert = l->vertices().size();
                glDrawArraysInstanced((mode == Mesh::TriangleStrip) ? GL_TRIANGLE_STRIP : GL_LINE_STRIP, 0, vert, count);
                PROFILER_STAT(POLYGONS, index - 2 * count);
            } else {
                uint32_t index = l->indices().size();
                glDrawElementsInstanced((mode == Mesh::Triangles) ? GL_TRIANGLES : GL_LINES, index, GL_UNSIGNED_INT, nullptr, count);
                PROFILER_STAT(POLYGONS, (index / 3) * count);
            }
            PROFILER_STAT(DRAWCALLS, 1);

            glBindVertexArray(0);
        }
    }
}

void CommandBufferGL::setRenderTarget(const TargetBuffer &target, RenderTexture *depth, uint32_t level) {
    PROFILE_FUNCTION();

    uint32_t colors[8];

    uint32_t buffer = 0;
    if(!target.empty()) {
        for(uint32_t i = 0; i < target.size(); i++) {
            RenderTextureGL *c = static_cast<RenderTextureGL *>(target[i]);
            if(i == 0) {
                buffer = c->buffer();
                glBindFramebuffer(GL_FRAMEBUFFER, buffer);
            }
            colors[i] = GL_COLOR_ATTACHMENT0 + i;
            glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, (uint32_t)(size_t)c->nativeHandle(), level );
        }
    }

    if(depth) {
        RenderTextureGL *t = static_cast<RenderTextureGL *>(depth);
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

Texture *CommandBufferGL::texture(const char *name) const {
    auto it = m_Textures.find(name);
    if(it != m_Textures.end()) {
        return (*it).second;
    }
    return nullptr;
}

void CommandBufferGL::setColor(const Vector4 &color) {
    m_Color = color;
}

void CommandBufferGL::resetViewProjection() {
    m_View = m_SaveView;
    m_Projection = m_SaveProjection;
}

void CommandBufferGL::setViewProjection(const Matrix4 &view, const Matrix4 &projection) {
    m_SaveView = m_View;
    m_SaveProjection= m_Projection;

    m_View = view;
    m_Projection = projection;
}

void CommandBufferGL::setGlobalValue(const char *name, const Variant &value) {
    m_Uniforms[name] = value;
}

void CommandBufferGL::setGlobalTexture(const char *name, Texture *value) {
    m_Textures[name] = value;
}

void CommandBufferGL::setViewport(int32_t x, int32_t y, int32_t width, int32_t height) {
    glViewport(x, y, width, height);
}

void CommandBufferGL::enableScissor(int32_t x, int32_t y, int32_t width, int32_t height) {
    glEnable(GL_SCISSOR_TEST);
    glScissor(x, y, width, height);
}

void CommandBufferGL::disableScissor() {
    glDisable(GL_SCISSOR_TEST);
}
