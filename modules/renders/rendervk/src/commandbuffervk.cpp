#include "commandbuffervk.h"

#include "resources/materialvk.h"
#include "resources/meshvk.h"
#include "resources/texturevk.h"
#include "resources/rendertargetvk.h"
#include "resources/computeshadervk.h"

#include "wrappervk.h"

#include <timer.h>
#include <log.h>

PFN_vkSetDebugUtilsObjectNameEXT CommandBufferVk::vkSetDebugUtilsObjectNameEXT;
PFN_vkCmdBeginDebugUtilsLabelEXT CommandBufferVk::vkCmdBeginDebugUtilsLabelEXT;
PFN_vkCmdEndDebugUtilsLabelEXT CommandBufferVk::vkCmdEndDebugUtilsLabelEXT;

CommandBufferVk::CommandBufferVk() :
        m_currentTarget(nullptr),
        m_commandBuffer(VK_NULL_HANDLE) {
    PROFILE_FUNCTION();

}

void CommandBufferVk::begin(VkCommandBuffer buffer) {
    PROFILE_FUNCTION();

    m_commandBuffer = buffer;

    m_global.time = Timer::deltaTime();
    m_global.clip = 0.1f;

    VkDevice device = WrapperVk::device();

    for(auto it : m_textures) {
        if(it.texture && it.texture->state() == Resource::ToBeUpdated) {
            TextureVk *texture = static_cast<TextureVk *>(it.texture);
            VkDescriptorImageInfo info = {};
            texture->attributes(info);
        }
    }

    for(auto it : m_suspendedSets) {
        vkFreeDescriptorSets(device, it.first, 1, &it.second);
    }
    m_suspendedSets.clear();

    vkResetCommandBuffer(m_commandBuffer, 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if(vkBeginCommandBuffer(m_commandBuffer, &beginInfo) != VK_SUCCESS) {
        aError() << "failed to begin recording command buffer!";
    }
}

void CommandBufferVk::end() {
    PROFILE_FUNCTION();

    if(m_currentTarget) {
        m_currentTarget->unbind(m_commandBuffer);
        m_currentTarget = nullptr;
    }

    if(vkEndCommandBuffer(m_commandBuffer) != VK_SUCCESS) {
        aError() << "failed to record command buffer!";
    }
}

void CommandBufferVk::clearRenderTarget(bool clearColor, const Vector4 &color, bool clearDepth, float depth) {
    PROFILE_FUNCTION();

    if(m_currentTarget) {
        m_currentTarget->clear(m_commandBuffer, clearColor, color, clearDepth, depth);
    }
}

void CommandBufferVk::dispatchCompute(ComputeInstance *shader, int32_t groupsX, int32_t groupsY, int32_t groupsZ) {
    PROFILE_FUNCTION();

    if(shader) {
        ComputeInstanceVk *comp = static_cast<ComputeInstanceVk *>(shader);
        if(comp->bind(*this)) {
            vkCmdDispatch(m_commandBuffer, groupsX, groupsY, groupsZ);
        }
    }
}

void CommandBufferVk::drawMesh(Mesh *mesh, uint32_t sub, uint32_t layer, MaterialInstance &instance) {
    PROFILE_FUNCTION();

    if(mesh) {
        MeshVk *meshVk = static_cast<MeshVk *>(mesh);

        MaterialInstanceVk &instanceVk = static_cast<MaterialInstanceVk &>(instance);
        if(instanceVk.bind(*this, layer, m_global)) {
            meshVk->bind(m_commandBuffer);

            if(meshVk->indices().empty()) {
                uint32_t vert = meshVk->vertices().size();
                vkCmdDraw(m_commandBuffer, vert, instance.instanceCount(), 0, 0);
                PROFILER_STAT(POLYGONS, vert - 2);
            } else {
                uint32_t index = meshVk->indexCount(sub);
                vkCmdDrawIndexed(m_commandBuffer, index, instance.instanceCount(), meshVk->indexStart(sub), 0, 0);
                PROFILER_STAT(POLYGONS, index / 3);
            }
            PROFILER_STAT(DRAWCALLS, 1);
        }
    }
}

void CommandBufferVk::setRenderTarget(RenderTarget *target, uint32_t level) {
    PROFILE_FUNCTION();

    if(m_currentTarget) {
        m_currentTarget->unbind(m_commandBuffer);
    }

    m_currentTarget = static_cast<RenderTargetVk *>(target);
    if(m_currentTarget) {
        m_currentTarget->bind(m_commandBuffer, level);
    }
}

VkCommandBuffer CommandBufferVk::nativeBuffer() const {
    return m_commandBuffer;
}

RenderTargetVk *CommandBufferVk::currentRenderTarget() const {
    return m_currentTarget;
}

void CommandBufferVk::suspendDescriptorSet(VkDescriptorPool pool, VkDescriptorSet set) {
    m_suspendedSets.push_back(std::make_pair(pool, set));
}

void CommandBufferVk::setViewport(int32_t x, int32_t y, int32_t width, int32_t height) {
    PROFILE_FUNCTION();

    CommandBuffer::setViewport(x, y, width, height);

    VkViewport viewport = {};
    viewport.x = (float)m_viewportX;
    viewport.y = (float)m_viewportY;
    viewport.width = (float)m_viewportWidth;
    viewport.height = (float)m_viewportHeight;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);

    enableScissor(x, y, width, height);
}

void CommandBufferVk::enableScissor(int32_t x, int32_t y, int32_t width, int32_t height) {
    PROFILE_FUNCTION();

    VkRect2D scissor = {};
    scissor.offset = { x, y };
    scissor.extent = { (uint32_t)width, (uint32_t)height };

    vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);
}

void CommandBufferVk::disableScissor() {
    PROFILE_FUNCTION();

    VkRect2D scissor = {};
    scissor.offset = {m_viewportX, m_viewportY};
    scissor.extent = {(uint32_t)m_viewportWidth, (uint32_t)m_viewportHeight};

    vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);
}

void CommandBufferVk::beginDebugMarker(const char *name) {
    static std::vector<Vector3> colors = {
        Vector3(1.0f, 0.4f, 0.4f),
        Vector3(0.4f, 1.0f, 0.4f),
        Vector3(0.4f, 0.4f, 1.0f),
        Vector3(1.0f, 1.0f, 0.4f),
        Vector3(1.0f, 0.4f, 1.0f),
        Vector3(0.4f, 1.0f, 1.0f),
    };

    static uint32_t index = 0;

    VkDebugUtilsLabelEXT label = {};
    label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
    label.pNext = nullptr;
    label.pLabelName = name;
    label.color[0] = colors[index].x;
    label.color[1] = colors[index].y;
    label.color[2] = colors[index].z;
    label.color[3] = 1.0f;

    index++;
    if(index >= colors.size()) {
        index = 0;
    }

    if(vkCmdBeginDebugUtilsLabelEXT == nullptr) {
        vkCmdBeginDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>((void*)vkGetDeviceProcAddr(WrapperVk::device(), "vkCmdBeginDebugUtilsLabelEXT"));
    }
    vkCmdBeginDebugUtilsLabelEXT(m_commandBuffer, &label);
}

void CommandBufferVk::endDebugMarker() {
    if(vkCmdEndDebugUtilsLabelEXT == nullptr) {
        vkCmdEndDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>((void*)vkGetDeviceProcAddr(WrapperVk::device(), "vkCmdEndDebugUtilsLabelEXT"));
    }
    vkCmdEndDebugUtilsLabelEXT(m_commandBuffer);
}
