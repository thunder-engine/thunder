#include "commandbuffergl.h"

#include <cstring>

#include "agl.h"

#include "resources/materialgl.h"
#include "resources/meshgl.h"
#include "resources/texturegl.h"
#include "resources/rendertargetgl.h"
#include "resources/pipeline.h"

#include <log.h>
#include <timer.h>

CommandBufferGL::CommandBufferGL() :
        m_globalUbo(0),
        m_localUbo(0) {
    PROFILE_FUNCTION();

}

CommandBufferGL::~CommandBufferGL() {

}

void CommandBufferGL::begin() {
    PROFILE_FUNCTION();

    if(m_globalUbo == 0) {
        glGenBuffers(1, &m_globalUbo);
        glBindBuffer(GL_UNIFORM_BUFFER, m_globalUbo);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(Global), nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    glBindBufferBase(GL_UNIFORM_BUFFER, GLOBAL_BIND, m_globalUbo);

    m_global.time = Timer::deltaTime();
    m_global.clip = 0.99f;

    glBindBuffer(GL_UNIFORM_BUFFER, m_globalUbo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Global), &m_globalUbo);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    if(m_localUbo == 0) {
        glGenBuffers(1, &m_localUbo);
        glBindBuffer(GL_UNIFORM_BUFFER, m_localUbo);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(Local), nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glBindBufferBase(GL_UNIFORM_BUFFER, LOCAL_BIND, m_localUbo);
    }
}

void CommandBufferGL::clearRenderTarget(bool clearColor, const Vector4 &color, bool clearDepth, float depth) {
    PROFILE_FUNCTION();

    uint32_t flags = 0;
    if(clearColor) {
        flags |= GL_COLOR_BUFFER_BIT;
        glClearColor(color.x, color.y, color.z, color.w);
    }
    if(clearDepth) {
        glDepthMask(GL_TRUE);
        flags |= GL_DEPTH_BUFFER_BIT;
        glClearDepthf(depth);
    }
    glClear(flags);
}

const VariantMap &CommandBufferGL::params() const {
    return m_uniforms;
}

void CommandBufferGL::drawMesh(const Matrix4 &model, Mesh *mesh, uint32_t sub, uint32_t layer, MaterialInstance *material) {
    PROFILE_FUNCTION();
    A_UNUSED(sub);

    if(mesh && material) {
        MeshGL *m = static_cast<MeshGL *>(mesh);
        uint32_t lod = 0;
        Lod *l = mesh->lod(lod);
        if(l == nullptr) {
            return;
        }

        m_local.model = model;

        glBindBuffer(GL_UNIFORM_BUFFER, m_localUbo);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Local), &m_local);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        MaterialInstanceGL *instance = static_cast<MaterialInstanceGL *>(material);
        if(instance->bind(this, layer)) {
            m->bindVao(this, lod);

            Mesh::TriangleTopology topology = static_cast<Mesh::TriangleTopology>(mesh->topology());
            if(topology > Mesh::Lines) {
                uint32_t vert = l->vertices().size();
                int32_t glMode = GL_TRIANGLE_STRIP;
                switch(topology) {
                case Mesh::LineStrip:   glMode = GL_LINE_STRIP; break;
                case Mesh::TriangleFan: glMode = GL_TRIANGLE_FAN; break;
                default: break;
                }
                glDrawArrays(glMode, 0, vert);
                PROFILER_STAT(POLYGONS, vert - 2);
            } else {
                uint32_t index = l->indices().size();
                glDrawElements((topology == Mesh::Triangles) ? GL_TRIANGLES : GL_LINES, index, GL_UNSIGNED_INT, nullptr);
                PROFILER_STAT(POLYGONS, index / 3);
            }
            PROFILER_STAT(DRAWCALLS, 1);

            glBindVertexArray(0);
        }
    }
}

void CommandBufferGL::drawMeshInstanced(const Matrix4 *models, uint32_t count, Mesh *mesh, uint32_t sub, uint32_t layer, MaterialInstance *material) {
    PROFILE_FUNCTION();
    A_UNUSED(sub);

    if(mesh && material) {
        MeshGL *m = static_cast<MeshGL *>(mesh);
        uint32_t lod = 0;
        Lod *l = mesh->lod(lod);
        if(l == nullptr) {
            return;
        }

        m_local.model.identity();

        glBindBuffer(GL_UNIFORM_BUFFER, m_localUbo);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Local), &m_local);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        MaterialInstanceGL *instance = static_cast<MaterialInstanceGL *>(material);
        if(instance->bind(this, layer)) {
            glBindBuffer(GL_ARRAY_BUFFER, m->instance());
            glBufferData(GL_ARRAY_BUFFER, count * sizeof(Matrix4), models, GL_DYNAMIC_DRAW);

            m->bindVao(this, lod);

            Mesh::TriangleTopology topology = static_cast<Mesh::TriangleTopology>(mesh->topology());
            if(topology > Mesh::Lines) {
                uint32_t vert = l->vertices().size();
                glDrawArraysInstanced((topology == Mesh::TriangleStrip) ? GL_TRIANGLE_STRIP : GL_LINE_STRIP, 0, vert, count);
                PROFILER_STAT(POLYGONS, index - 2 * count);
            } else {
                uint32_t index = l->indices().size();
                glDrawElementsInstanced((topology == Mesh::Triangles) ? GL_TRIANGLES : GL_LINES, index, GL_UNSIGNED_INT, nullptr, count);
                PROFILER_STAT(POLYGONS, (index / 3) * count);
            }
            PROFILER_STAT(DRAWCALLS, 1);

            glBindVertexArray(0);
        }
    }
}

void CommandBufferGL::setRenderTarget(RenderTarget *target, uint32_t level) {
    PROFILE_FUNCTION();

    RenderTargetGL *t = static_cast<RenderTargetGL *>(target);
    if(t) {
        t->bindBuffer(level);
    }
}

Texture *CommandBufferGL::texture(const char *name) const {
    PROFILE_FUNCTION();

    for(auto &it : m_textures) {
        if(it.name == name) {
            return it.texture;
        }
    }
    return nullptr;
}

void CommandBufferGL::resetViewProjection() {
    PROFILE_FUNCTION();

    CommandBuffer::resetViewProjection();

    glBindBuffer(GL_UNIFORM_BUFFER, m_globalUbo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Global), &m_global);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void CommandBufferGL::setViewProjection(const Matrix4 &view, const Matrix4 &projection) {
    PROFILE_FUNCTION();

    CommandBuffer::setViewProjection(view, projection);

    glBindBuffer(GL_UNIFORM_BUFFER, m_globalUbo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Global), &m_global);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void CommandBufferGL::setGlobalValue(const char *name, const Variant &value) {
    PROFILE_FUNCTION();

    static const unordered_map<string, pair<size_t, size_t>> offsets = {
        {"camera.position",      make_pair(offsetof(Global, cameraPosition),      sizeof(Global::cameraPosition))},
        {"camera.target",        make_pair(offsetof(Global, cameraTarget),        sizeof(Global::cameraTarget))},
        {"camera.view",          make_pair(offsetof(Global, cameraView),          sizeof(Global::cameraView))},
        {"camera.projectionInv", make_pair(offsetof(Global, cameraProjectionInv), sizeof(Global::cameraProjectionInv))},
        {"camera.projection",    make_pair(offsetof(Global, cameraProjection),    sizeof(Global::cameraProjection))},
        {"camera.screenToWorld", make_pair(offsetof(Global, cameraScreenToWorld), sizeof(Global::cameraScreenToWorld))},
        {"camera.worldToScreen", make_pair(offsetof(Global, cameraWorldToScreen), sizeof(Global::cameraWorldToScreen))},
        {"camera.screen",        make_pair(offsetof(Global, cameraScreen),        sizeof(Global::cameraScreen))},
        {"light.pageSize",       make_pair(offsetof(Global, lightPageSize),       sizeof(Global::lightPageSize))},
        {"light.ambient",        make_pair(offsetof(Global, lightAmbient),        sizeof(Global::lightAmbient))},
    };

    auto it = offsets.find(name);
    if(it != offsets.end()) {
        void *src = value.data();
        memcpy((uint8_t *)&m_global + it->second.first, value.data(), it->second.second);

        glBindBuffer(GL_UNIFORM_BUFFER, m_globalUbo);
        glBufferSubData(GL_UNIFORM_BUFFER, it->second.first, it->second.second, src);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    } else {
        m_uniforms[name] = value;
    }
}

void CommandBufferGL::setGlobalTexture(const char *name, Texture *texture) {
    PROFILE_FUNCTION();

    for(auto &it : m_textures) {
        if(it.name == name) {
            it.texture = texture;
            return;
        }
    }

    Material::TextureItem item;
    item.name = name;
    item.texture = texture;
    item.binding = -1;
    m_textures.push_back(item);
}

void CommandBufferGL::setViewport(int32_t x, int32_t y, int32_t width, int32_t height) {
    glViewport(x, y, width, height);
    setGlobalValue("camera.screen", Vector4(1.0f / (float)width, 1.0f / (float)height, width, height));
}

void CommandBufferGL::enableScissor(int32_t x, int32_t y, int32_t width, int32_t height) {
    glEnable(GL_SCISSOR_TEST);
    glScissor(x, y, width, height);
}

void CommandBufferGL::disableScissor() {
    glDisable(GL_SCISSOR_TEST);
}

void CommandBufferGL::finish() {
    glFinish();
}
