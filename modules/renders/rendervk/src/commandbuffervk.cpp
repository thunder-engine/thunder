#include "commandbuffervk.h"

#include "resources/materialvk.h"
#include "resources/meshvk.h"
#include "resources/texturevk.h"
#include "resources/rendertargetvk.h"
#include "resources/computeshadervk.h"

#include "wrappervk.h"

#include <timer.h>
#include <log.h>

PFN_vkCmdBeginDebugUtilsLabelEXT CommandBufferVk::vkCmdBeginDebugUtilsLabelEXT;
PFN_vkCmdEndDebugUtilsLabelEXT CommandBufferVk::vkCmdEndDebugUtilsLabelEXT;

CommandBufferVk::CommandBufferVk() :
        m_commandBuffer(VK_NULL_HANDLE) {
    PROFILE_FUNCTION();

}

CommandBufferVk::~CommandBufferVk() {

}

void CommandBufferVk::begin(VkCommandBuffer buffer) {
    PROFILE_FUNCTION();

    m_commandBuffer = buffer;
    m_currentFrame = (m_currentFrame + 1) % WrapperVk::framesInFlight();

    CommandBuffer::begin();

    VkDevice device = WrapperVk::device();

    for(auto &it : m_textures) {
        if(it.texture && it.texture->state() == Resource::ToBeUpdated) {
            TextureVk *texture = static_cast<TextureVk *>(it.texture);
            VkDescriptorImageInfo info = {};
            texture->attributes(info);
        }
    }

    for(auto it : m_suspended) {
        vkDestroyBuffer(device, it.first, nullptr);
        vkFreeMemory(device, it.second, nullptr);
    }
    m_suspended.clear();

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

    RenderTargetVk *targetVk = static_cast<RenderTargetVk *>(m_target);
    if(targetVk) {
        targetVk->unbind(m_commandBuffer);
        targetVk = nullptr;
    }

    if(vkEndCommandBuffer(m_commandBuffer) != VK_SUCCESS) {
        aError() << "failed to record command buffer!";
    }
}

size_t CommandBufferVk::currentFame() const {
    return m_currentFrame;
}

void CommandBufferVk::dispatchCompute(ComputeInstance &shader, int32_t groupsX, int32_t groupsY, int32_t groupsZ) {
    PROFILE_FUNCTION();

    ComputeInstanceVk &comp = static_cast<ComputeInstanceVk &>(shader);
    if(comp.bind(*this)) {
        vkCmdDispatch(m_commandBuffer, groupsX, groupsY, groupsZ);
    }
}

void CommandBufferVk::drawMesh(Mesh *mesh, uint32_t sub, uint32_t layer, MaterialInstance &instance) {
    PROFILE_FUNCTION();

    if(mesh) {
        MeshVk *meshVk = static_cast<MeshVk *>(mesh);

        MaterialInstanceVk &instanceVk = static_cast<MaterialInstanceVk &>(instance);
        if(instanceVk.bind(*this, layer, static_cast<RenderTargetVk *>(m_target)->globalDescriptorSet(m_currentFrame), m_currentFrame)) {
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

    RenderTargetVk *targetVk = static_cast<RenderTargetVk *>(m_target);
    if(targetVk) {
        targetVk->unbind(m_commandBuffer);
    }

    CommandBuffer::setRenderTarget(target, level);

    targetVk = static_cast<RenderTargetVk *>(m_target);
    if(targetVk) {
        targetVk->bind(m_commandBuffer, level);
        if(!(targetVk->flags() & RenderTarget::Atlas)) {
            targetVk->updateGlobalMemory(m_currentFrame, m_global);
        }
    }
}

VkCommandBuffer CommandBufferVk::nativeBuffer() const {
    return m_commandBuffer;
}

void CommandBufferVk::suspendBuffer(VkBuffer buffer, VkDeviceMemory memory) {
    if(buffer != VK_NULL_HANDLE && memory != VK_NULL_HANDLE) {
        m_suspended.push_back(std::make_pair(buffer, memory));
    }
}

std::vector<VkDescriptorSetLayoutBinding> &CommandBufferVk::globalLayoutBindings() {
    static std::vector<VkDescriptorSetLayoutBinding> s_globalLayoutBindings;
    s_globalLayoutBindings = {
        { GLOBAL_BIND, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr }
    };
    return s_globalLayoutBindings;
}

VkDescriptorSetLayout CommandBufferVk::globalDescriptorSetLayout() {
    static VkDescriptorSetLayout s_globalDescSetLayout = WrapperVk::createDescriptorSetLayout(globalLayoutBindings());
    return s_globalDescSetLayout;
}

void CommandBufferVk::setViewport(int32_t x, int32_t y, int32_t width, int32_t height) {
    PROFILE_FUNCTION();

    CommandBuffer::setViewport(x, y, width, height);

    if(m_target && m_target->flags() & RenderTarget::Atlas && m_target->tileIndex() >= 0) {
        static_cast<RenderTargetVk *>(m_target)->updateGlobalMemory(m_currentFrame, m_global);
    }

    m_viewport.x = (float)x;
    m_viewport.y = (float)y;
    m_viewport.width = (float)width;
    m_viewport.height = (float)height;
    m_viewport.minDepth = 0.0f;
    m_viewport.maxDepth = 1.0f;

    vkCmdSetViewport(m_commandBuffer, 0, 1, &m_viewport);

    enableScissor(x, y, width, height);
}

void CommandBufferVk::enableScissor(int32_t x, int32_t y, int32_t width, int32_t height) {
    PROFILE_FUNCTION();

    CommandBuffer::enableScissor(x, y, width, height);

    VkRect2D scissor = {};
    scissor.offset = { x, y };
    scissor.extent = { (uint32_t)width, (uint32_t)height };

    vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);
}

void CommandBufferVk::disableScissor() {
    PROFILE_FUNCTION();

    VkRect2D scissor = {};

    CommandBuffer::disableScissor();
    if(m_scissorStack.empty()) {
        scissor.offset = {(int32_t)m_viewport.x, (int32_t)m_viewport.y};
        scissor.extent = {(uint32_t)m_viewport.width, (uint32_t)m_viewport.height};
    } else {
        ScissorRect rect = m_scissorStack.top(); // Get previous
        scissor.offset = {rect.x, rect.y};
        scissor.extent = {(uint32_t)rect.width, (uint32_t)rect.height};
    }

    vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);
}

void CommandBufferVk::flipResult() {
    m_global.params.w = 1.0f;
}

void CommandBufferVk::beginDebugMarker(const TString &name) {
    static const std::vector<Vector3> colors = {
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
    label.pLabelName = name.data();
    label.color[0] = colors[index].x;
    label.color[1] = colors[index].y;
    label.color[2] = colors[index].z;
    label.color[3] = 1.0f;

    index++;
    if(index >= colors.size()) {
        index = 0;
    }

    if(vkCmdBeginDebugUtilsLabelEXT == nullptr) {
        vkCmdBeginDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(vkGetDeviceProcAddr(WrapperVk::device(), "vkCmdBeginDebugUtilsLabelEXT"));
    }
    vkCmdBeginDebugUtilsLabelEXT(m_commandBuffer, &label);
}

void CommandBufferVk::endDebugMarker() {
    if(vkCmdEndDebugUtilsLabelEXT == nullptr) {
        vkCmdEndDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(vkGetDeviceProcAddr(WrapperVk::device(), "vkCmdEndDebugUtilsLabelEXT"));
    }
    vkCmdEndDebugUtilsLabelEXT(m_commandBuffer);
}
