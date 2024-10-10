#include "commandbuffergl.h"

#include <cstring>

#include "agl.h"

#include "resources/meshgl.h"
#include "resources/materialgl.h"
#include "resources/rendertargetgl.h"
#include "resources/computeshadergl.h"

#include <log.h>
#include <timer.h>

CommandBufferGL::CommandBufferGL() {

    PROFILE_FUNCTION();
}

void CommandBufferGL::begin() {
    PROFILE_FUNCTION();

    m_global.time = Timer::time();
    m_global.deltaTime = Timer::deltaTime();
    m_global.clip = 0.1f;
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
        flags |= GL_STENCIL_BUFFER_BIT;
        glClearDepthf(depth);
    }
    glClear(flags);
}

void CommandBufferGL::dispatchCompute(ComputeInstance *shader, int32_t groupsX, int32_t groupsY, int32_t groupsZ) {
#ifndef THUNDER_MOBILE
    PROFILE_FUNCTION();
    if(shader) {
        ComputeInstanceGL *instance = static_cast<ComputeInstanceGL *>(shader);
        if(instance->bind(this)) {
            glDispatchCompute(groupsX, groupsY, groupsZ);

            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        }
    }
#endif
}

void CommandBufferGL::drawMesh(Mesh *mesh, uint32_t sub, uint32_t layer, MaterialInstance &instance) {
    PROFILE_FUNCTION();

    if(mesh) {
        MeshGL *meshGL = static_cast<MeshGL *>(mesh);

        MaterialInstanceGL &instanceGL = static_cast<MaterialInstanceGL &>(instance);
        for(uint32_t index = 0; index < instanceGL.drawsCount(); index++) {
            if(instanceGL.bind(this, layer, index, m_global)) {
                meshGL->bindVao(this);

                if(meshGL->indices().empty()) {
                    int32_t glMode = (instance.material()->wireframe()) ? GL_LINE_STRIP : GL_TRIANGLE_STRIP;
                    uint32_t vert = meshGL->vertices().size();
                    glDrawArraysInstanced(glMode, 0, vert, instance.instanceCount());
                    PROFILER_STAT(POLYGONS, index - 2 * count);
                } else {
                    int32_t index = meshGL->indexCount(sub);
                    int32_t glMode = (instance.material()->wireframe()) ? GL_LINES : GL_TRIANGLES;
                    glDrawElementsInstanced(glMode, index, GL_UNSIGNED_INT, reinterpret_cast<void *>(meshGL->indexStart(sub) * sizeof(int32_t)), instance.instanceCount());
                    PROFILER_STAT(POLYGONS, (index / 3) * count);
                }
                PROFILER_STAT(DRAWCALLS, 1);

                glBindVertexArray(0);
            }
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

void CommandBufferGL::setViewProjection(const Matrix4 &view, const Matrix4 &projection) {
    PROFILE_FUNCTION();

    CommandBuffer::setViewProjection(view, projection);
}

void CommandBufferGL::setViewport(int32_t x, int32_t y, int32_t width, int32_t height) {
    CommandBuffer::setViewport(x, y, width, height);

    glViewport(x, y, width, height);
}

void CommandBufferGL::enableScissor(int32_t x, int32_t y, int32_t width, int32_t height) {
    glEnable(GL_SCISSOR_TEST);
    glScissor(x, y, width, height);
}

void CommandBufferGL::disableScissor() {
    glDisable(GL_SCISSOR_TEST);
}

void CommandBufferGL::beginDebugMarker(const char *name) {
#ifndef THUNDER_MOBILE
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, strlen(name), name);
#endif
}

void CommandBufferGL::endDebugMarker() {
#ifndef THUNDER_MOBILE
    glPopDebugGroup();
#endif
}

void CommandBufferGL::setObjectName(int32_t type, int32_t id, const std::string &name) {
#ifndef THUNDER_MOBILE
    glObjectLabel(type, id, name.size(), name.c_str());
#endif
}
