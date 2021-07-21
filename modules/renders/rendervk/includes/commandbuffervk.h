#ifndef COMMANDBUFFERVK_H
#define COMMANDBUFFERVK_H

#include <commandbuffer.h>

#include "resources/materialvk.h"
#include "resources/meshvk.h"

#define VERTEX_ATRIB    0
#define UV0_ATRIB       1
#define NORMAL_ATRIB    2
#define TANGENT_ATRIB   3
#define COLOR_ATRIB     4

#define UV1_ATRIB       5
#define BONES_ATRIB     6
#define WEIGHTS_ATRIB   7

#define INSTANCE_ATRIB  8

class CommandBufferVk : public ICommandBuffer {
    A_OVERRIDE(CommandBufferVk, ICommandBuffer, System)

public:
    CommandBufferVk();

    ~CommandBufferVk() override;

    void begin(VkCommandBuffer buffer, VkImage colorImage, VkImage depthImage, uint32_t index);

    static void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &memory);

    static VkDescriptorSetLayout createDescriptorSetLayout(const vector<VkDescriptorSetLayoutBinding> &layoutBinding);

    static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    static VkCommandBuffer beginSingleTimeCommands();

    static void endSingleTimeCommands(VkCommandBuffer commandBuffer);

private:
    void clearRenderTarget(bool clearColor = true, const Vector4 &color = Vector4(0.0f), bool clearDepth = true, float depth = 1.0f) override;

    void drawMesh(const Matrix4 &model, Mesh *mesh, uint32_t layer = ICommandBuffer::DEFAULT, MaterialInstance *material = nullptr) override;

    void drawMeshInstanced(const Matrix4 *models, uint32_t count, Mesh *mesh, uint32_t layer = ICommandBuffer::DEFAULT, MaterialInstance *material = nullptr) override;

    void setRenderTarget(RenderTarget *target, uint32_t level = 0) override;

    void setRenderTarget(uint32_t target) override;

    void setColor(const Vector4 &color) override;

    void resetViewProjection() override;

    void setViewProjection(const Matrix4 &view, const Matrix4 &projection) override;

    void setGlobalValue(const char *name, const Variant &value) override;

    void setGlobalTexture(const char *name, Texture *value) override;

    void setViewport(int32_t x, int32_t y, int32_t width, int32_t height) override;

    void enableScissor(int32_t x, int32_t y, int32_t width, int32_t height) override;

    void disableScissor() override;

    Matrix4 projection() const override;

    Matrix4 view() const override;

    Texture *texture(const char *name) const override;

protected:
    VertexBufferObject m_vertex;
    FragmentBufferObject m_fragment;

    Matrix4 m_saveView;
    Matrix4 m_saveProjection;

    VkCommandBuffer m_commandBuffer;
    VkImage m_currentColorImage;
    VkImage m_currentDepthImage;
    uint32_t m_currentImageIndex;

    int32_t m_viewportX;
    int32_t m_viewportY;
    int32_t m_viewportWidth;
    int32_t m_viewportHeight;
};

#endif // COMMANDBUFFERVK_H
