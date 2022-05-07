#ifndef COMMANDBUFFERVK_H
#define COMMANDBUFFERVK_H

#include <commandbuffer.h>

#include <vulkan/vulkan.h>

#define VERTEX_ATRIB    0
#define UV0_ATRIB       1
#define NORMAL_ATRIB    2
#define TANGENT_ATRIB   3
#define COLOR_ATRIB     4

class RenderTargetVk;

class CommandBufferVk : public CommandBuffer {
    A_OVERRIDE(CommandBufferVk, CommandBuffer, System)

public:
    CommandBufferVk();

    void begin(VkCommandBuffer buffer, uint32_t index);

    void end();

    VkCommandBuffer nativeBuffer() const;

    RenderTargetVk *currentRenderTarget() const;

    static void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &memory);

    static VkDescriptorSetLayout createDescriptorSetLayout(const vector<VkDescriptorSetLayoutBinding> &layoutBinding);

    static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    static VkCommandBuffer beginSingleTimeCommands();

    static void endSingleTimeCommands(VkCommandBuffer commandBuffer);

private:
    void clearRenderTarget(bool clearColor = true, const Vector4 &color = Vector4(0.0f), bool clearDepth = true, float depth = 1.0f) override;

    void drawMesh(const Matrix4 &model, Mesh *mesh, uint32_t sub, uint32_t layer = CommandBuffer::DEFAULT, MaterialInstance *material = nullptr) override;

    void drawMeshInstanced(const Matrix4 *models, uint32_t count, Mesh *mesh, uint32_t sub, uint32_t layer = CommandBuffer::DEFAULT, MaterialInstance *material = nullptr) override;

    void setRenderTarget(RenderTarget *target, uint32_t level = 0) override;

    void setScreenProjection(float l = -0.5f, float t = -0.5f, float r = 0.5f, float b = 0.5f) override;

    void setViewport(int32_t x, int32_t y, int32_t width, int32_t height) override;

    void enableScissor(int32_t x, int32_t y, int32_t width, int32_t height) override;

    void disableScissor() override;

protected:
    Matrix4 m_saveView;
    Matrix4 m_saveProjection;

    VkCommandBuffer m_commandBuffer;
    uint32_t m_currentImageIndex;

    RenderTargetVk *m_currentTarget;

};

#endif // COMMANDBUFFERVK_H
