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

VkDescriptorSetLayout CommandBufferVk::s_globalDescriptorSetLayout = VK_NULL_HANDLE;
vector<VkDescriptorSetLayoutBinding> CommandBufferVk::s_globalLayoutBindings;

CommandBufferVk::CommandBufferVk() :
        m_currentTarget(nullptr),
        m_commandBuffer(VK_NULL_HANDLE),
        m_globalDescriptorSet(VK_NULL_HANDLE),
        m_globalBuffer(VK_NULL_HANDLE),
        m_globalBufferMemory(VK_NULL_HANDLE) {
    PROFILE_FUNCTION();

}

void CommandBufferVk::begin(VkCommandBuffer buffer) {
    PROFILE_FUNCTION();

    m_commandBuffer = buffer;

    m_global.time = Timer::deltaTime();
    m_global.clip = 0.1f;

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

void CommandBufferVk::setViewProjection(const Matrix4 &view, const Matrix4 &projection) {
    PROFILE_FUNCTION();

    CommandBuffer::setViewProjection(view, projection);

    if(m_globalBufferMemory) {
        VkDevice device = WrapperVk::device();

        void *dst = nullptr;
        vkMapMemory(device, m_globalBufferMemory, 0, sizeof(Global), 0, &dst);
            memcpy(dst, &m_global, sizeof(Global));
        vkUnmapMemory(device, m_globalBufferMemory);
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
        if(instanceVk.bind(*this, layer)) {
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

VkDescriptorSet CommandBufferVk::globalDescriptorSet() {
    PROFILE_FUNCTION();

    VkDevice device = WrapperVk::device();

    if(m_globalDescriptorSet == VK_NULL_HANDLE) {
        uint32_t swapChainCount = 1;
        // Descriptor pool
        vector<VkDescriptorPoolSize> poolSize;
        for(auto &binding : s_globalLayoutBindings) {
            poolSize.push_back({ binding.descriptorType, (uint32_t)swapChainCount });
        }

        m_globalDescriptorPool = WrapperVk::createDescriptorPool(poolSize, swapChainCount);
        m_globalDescriptorSet = WrapperVk::createDescriptorSet(s_globalDescriptorSetLayout, m_globalDescriptorPool);

        // Create descriptor buffers
        for(auto &binding : s_globalLayoutBindings) {
            VkWriteDescriptorSet descriptorWrite = {};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = m_globalDescriptorSet;
            descriptorWrite.dstBinding = binding.binding;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = binding.descriptorType;
            descriptorWrite.descriptorCount = 1;

            if(binding.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                WrapperVk::createBuffer(sizeof(Global), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, m_globalBuffer);
                WrapperVk::allocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_globalBuffer, m_globalBufferMemory);

                VkDescriptorBufferInfo bufferInfo = {};
                bufferInfo.buffer = m_globalBuffer;
                bufferInfo.offset = 0;
                bufferInfo.range = VK_WHOLE_SIZE;

                descriptorWrite.pBufferInfo = &bufferInfo;

                vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
            }
        }
    }

    return m_globalDescriptorSet;
}

VkDescriptorSetLayout CommandBufferVk::globalDescriptorSetLayout() {
    PROFILE_FUNCTION();

    if(s_globalDescriptorSetLayout == VK_NULL_HANDLE) {
        s_globalLayoutBindings = {
            { GLOBAL_BIND, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr }
        };

        s_globalDescriptorSetLayout = WrapperVk::createDescriptorSetLayout(s_globalLayoutBindings);
    }
    return s_globalDescriptorSetLayout;
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
    static vector<Vector3> colors = {
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
        vkCmdBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT)(void*)vkGetDeviceProcAddr(WrapperVk::device(), "vkCmdBeginDebugUtilsLabelEXT");
    }
    vkCmdBeginDebugUtilsLabelEXT(m_commandBuffer, &label);
}

void CommandBufferVk::endDebugMarker() {
    if(vkCmdEndDebugUtilsLabelEXT == nullptr) {
        vkCmdEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT)(void*)vkGetDeviceProcAddr(WrapperVk::device(), "vkCmdEndDebugUtilsLabelEXT");
    }
    vkCmdEndDebugUtilsLabelEXT(m_commandBuffer);
}
