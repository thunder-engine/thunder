#ifndef COMMANDBUFFERVK_H
#define COMMANDBUFFERVK_H

#include <commandbuffer.h>

#include <vulkan/vulkan.h>

#define VERTEX_ATRIB    0
#define UV0_ATRIB       1
#define COLOR_ATRIB     2
#define NORMAL_ATRIB    3
#define TANGENT_ATRIB   4

class RenderTargetVk;

class CommandBufferVk : public CommandBuffer {
    A_OVERRIDE(CommandBufferVk, CommandBuffer, System)

public:
    CommandBufferVk();

    void begin(VkCommandBuffer buffer);

    void end();

    VkCommandBuffer nativeBuffer() const;

    RenderTargetVk *currentRenderTarget() const;

    VkDescriptorSet globalDescriptorSet();

    static VkDescriptorSetLayout globalDescriptorSetLayout();

private:
    void beginDebugMarker(const char *name) override;
    void endDebugMarker() override;

    void setViewProjection(const Matrix4 &view, const Matrix4 &projection) override;

    void clearRenderTarget(bool clearColor, const Vector4 &color, bool clearDepth, float depth) override;

    void dispatchCompute(ComputeInstance *shader, int32_t groupsX, int32_t groupsY, int32_t groupsZ) override;

    void drawMesh(Mesh *mesh, uint32_t sub, uint32_t layer, MaterialInstance &instance) override;

    void setRenderTarget(RenderTarget *target, uint32_t level = 0) override;

    void setViewport(int32_t x, int32_t y, int32_t width, int32_t height) override;

    void enableScissor(int32_t x, int32_t y, int32_t width, int32_t height) override;

    void disableScissor() override;

protected:
    Matrix4 m_saveView;
    Matrix4 m_saveProjection;

    RenderTargetVk *m_currentTarget;

    VkCommandBuffer m_commandBuffer;

    VkDescriptorSet m_globalDescriptorSet;

    VkDescriptorPool m_globalDescriptorPool;

    VkBuffer m_globalBuffer;
    VkDeviceMemory m_globalBufferMemory;

    static PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT;
    static PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT;
    static PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT;

    static VkDescriptorSetLayout s_globalDescriptorSetLayout;

    static vector<VkDescriptorSetLayoutBinding> s_globalLayoutBindings;

};

#endif // COMMANDBUFFERVK_H
