#include "commandbuffervk.h"

#include "resources/materialvk.h"
#include "resources/meshvk.h"
#include "resources/texturevk.h"
#include "resources/rendertargetvk.h"

#include "rendervksystem.h"

#include <log.h>
#include <timer.h>

#include <array>

CommandBufferVk::CommandBufferVk() :
        m_commandBuffer(nullptr),
        m_currentColorImage(nullptr),
        m_currentDepthImage(nullptr),
        m_currentImageIndex(0),
        m_viewportX(0),
        m_viewportY(0),
        m_viewportWidth(1),
        m_viewportHeight(1) {
    PROFILE_FUNCTION();

    m_fragment.m_Clip = 0.1f;
    m_fragment.m_Time = 0.0f;

}

CommandBufferVk::~CommandBufferVk() {

}

void CommandBufferVk::begin(VkCommandBuffer buffer, VkImage colorImage, VkImage depthImage, uint32_t index) {
    m_commandBuffer = buffer;
    m_currentColorImage = colorImage;
    m_currentDepthImage = depthImage;
    m_currentImageIndex = index;
}

void CommandBufferVk::clearRenderTarget(bool clearColor, const Vector4 &color, bool clearDepth, float depth) {
    PROFILE_FUNCTION();

    if(clearColor) {
        VkClearColorValue clearColorValue = {{ color.x, color.y, color.z, color.w }};
        VkImageSubresourceRange range;
        range.layerCount = 1;
        range.levelCount = 1;
        range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        range.baseArrayLayer = 0;
        range.baseMipLevel = 0;
        vkCmdClearColorImage(m_commandBuffer, m_currentColorImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColorValue, 1, &range);
    }
    if(clearDepth) {
        VkClearDepthStencilValue clearDS = { depth, 0 };
        VkImageSubresourceRange range;
        range.layerCount = 1;
        range.levelCount = 1;
        range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        range.baseArrayLayer = 0;
        range.baseMipLevel = 0;
        vkCmdClearDepthStencilImage(m_commandBuffer, m_currentDepthImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearDS, 1, &range);
    }
}

void CommandBufferVk::drawMesh(const Matrix4 &model, Mesh *mesh, uint32_t layer, MaterialInstance *material) {
    PROFILE_FUNCTION();

    drawMeshInstanced(&model, 1, mesh, layer, material);
}

void CommandBufferVk::drawMeshInstanced(const Matrix4 *models, uint32_t count, Mesh *mesh, uint32_t layer, MaterialInstance *material) {
    PROFILE_FUNCTION();

    if(mesh && material) {
        MeshVk *m = static_cast<MeshVk *>(mesh);
        uint32_t lod = 0;
        Lod *l = mesh->lod(lod);
        if(l == nullptr) {
            return;
        }

        m_vertex.m_Model = models[0];

        MaterialInstanceVk *mat = static_cast<MaterialInstanceVk *>(material);
        if(mat->bind(m_vertex, m_fragment, m_commandBuffer, m_currentImageIndex, layer)) {
            m->bind(m_commandBuffer, lod);
            Mesh::Topology mode = static_cast<Mesh::Topology>(mesh->topology());
            if(mode > Mesh::Lines) {
                uint32_t vert = l->vertices().size();
                vkCmdDraw(m_commandBuffer, vert, count, 0, 0);
                PROFILER_STAT(POLYGONS, vert - 2);
            } else {
                uint32_t index = l->indices().size();
                vkCmdDrawIndexed(m_commandBuffer, index, count, 0, 0, 0);
                PROFILER_STAT(POLYGONS, index / 3);
            }
            PROFILER_STAT(DRAWCALLS, 1);
        }
    }
}

void CommandBufferVk::setRenderTarget(RenderTarget *target, uint32_t level) {
    PROFILE_FUNCTION();

    RenderTargetVk *t = static_cast<RenderTargetVk *>(target);
    if(t) {
        t->bindTarget(m_commandBuffer, level);
    }
}

void CommandBufferVk::setRenderTarget(uint32_t target) {
    PROFILE_FUNCTION();
}

Matrix4 CommandBufferVk::projection() const {
    return m_vertex.m_Projection;
}

Matrix4 CommandBufferVk::view() const {
    return m_vertex.m_View;
}

Texture *CommandBufferVk::texture(const char *name) const {
    return nullptr;
}

void CommandBufferVk::setColor(const Vector4 &color) {
    m_fragment.m_Color = color;
}

void CommandBufferVk::resetViewProjection() {
    m_vertex.m_View = m_saveView;
    m_vertex.m_Projection = m_saveProjection;
}

void CommandBufferVk::setViewProjection(const Matrix4 &view, const Matrix4 &projection) {
    m_saveView = m_vertex.m_View;
    m_saveProjection = m_vertex.m_Projection;

    m_vertex.m_View = view;
    m_vertex.m_Projection = projection;
    m_vertex.m_Projection.mat[5] *= -1.0f; // Inverse Y coordinate for Vulkan
}

void CommandBufferVk::setGlobalValue(const char *name, const Variant &value) {

}

void CommandBufferVk::setGlobalTexture(const char *name, Texture *value) {

}

void CommandBufferVk::setViewport(int32_t x, int32_t y, int32_t width, int32_t height) {
    m_viewportX = x;
    m_viewportY = y;
    m_viewportWidth = width;
    m_viewportHeight = height;

    VkViewport viewport = {};
    viewport.x = (float)m_viewportX;
    viewport.y = (float)m_viewportY;
    viewport.width = (float)m_viewportWidth;
    viewport.height = (float)m_viewportHeight;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);
}

void CommandBufferVk::enableScissor(int32_t x, int32_t y, int32_t width, int32_t height) {
    VkRect2D scissor = {};
    scissor.offset = {x, y};
    scissor.extent.width = width;
    scissor.extent.height = height;

    vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);
}

void CommandBufferVk::disableScissor() {
    VkRect2D scissor = {};
    scissor.offset = {m_viewportX, m_viewportY};
    scissor.extent.width = m_viewportWidth;
    scissor.extent.height = m_viewportHeight;

    vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);
}

void CommandBufferVk::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &memory) {
    VkDevice device = RenderVkSystem::currentDevice();

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if(vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw runtime_error("failed to crate buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if(vkAllocateMemory(device, &allocInfo, nullptr, &memory) != VK_SUCCESS) {
        throw runtime_error("failed to allocate memory!");
    }

    vkBindBufferMemory(device, buffer, memory, 0);
}

VkDescriptorSetLayout CommandBufferVk::createDescriptorSetLayout(const vector<VkDescriptorSetLayoutBinding> &layoutBinding) {
    VkDevice device = RenderVkSystem::currentDevice();

    VkDescriptorSetLayoutCreateInfo descLayoutInfo = {};
    descLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descLayoutInfo.bindingCount = layoutBinding.size();
    descLayoutInfo.pBindings = layoutBinding.data();

    VkDescriptorSetLayout descSetLayout;
    if(vkCreateDescriptorSetLayout(device, &descLayoutInfo, nullptr, &descSetLayout) != VK_SUCCESS) {
        throw runtime_error("failed to create descriptor set layout!");
    }
    return descSetLayout;
}

uint32_t CommandBufferVk::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(RenderVkSystem::currentPhysicalDevice(), &memProperties);

    for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    return 0;
}

VkCommandBuffer CommandBufferVk::beginSingleTimeCommands() {
    VkDevice device = RenderVkSystem::currentDevice();

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = RenderVkSystem::currentCommandPool();
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void CommandBufferVk::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    VkDevice device = RenderVkSystem::currentDevice();

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkQueue graphicsQueue = RenderVkSystem::currentQueue();

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(device, RenderVkSystem::currentCommandPool(), 1, &commandBuffer);
}
