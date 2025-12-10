#ifndef COMMANDBUFFERVK_H
#define COMMANDBUFFERVK_H

#include <commandbuffer.h>

#include <vulkan/vulkan.h>

class RenderTargetVk;

class CommandBufferVk : public CommandBuffer {
    A_OBJECT_OVERRIDE(CommandBufferVk, CommandBuffer, System)

public:
    CommandBufferVk();

    void begin(VkCommandBuffer buffer);

    void end();

    VkCommandBuffer nativeBuffer() const;

    RenderTargetVk *currentRenderTarget() const;

    void suspendDescriptorSet(VkDescriptorPool pool, VkDescriptorSet set);

private:
    void beginDebugMarker(const TString &name) override;
    void endDebugMarker() override;

    void dispatchCompute(ComputeInstance &shader, int32_t groupsX, int32_t groupsY, int32_t groupsZ) override;

    void drawMesh(Mesh *mesh, uint32_t sub, uint32_t layer, MaterialInstance &instance) override;

    void setRenderTarget(RenderTarget *target, uint32_t level = 0) override;

    void setViewport(int32_t x, int32_t y, int32_t width, int32_t height) override;

    void enableScissor(int32_t x, int32_t y, int32_t width, int32_t height) override;

    void disableScissor() override;

protected:
    RenderTargetVk *m_currentTarget;

    VkCommandBuffer m_commandBuffer;

    VkViewport m_viewport;

    static PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT;
    static PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT;
    static PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT;

    std::list<std::pair<VkDescriptorPool, VkDescriptorSet>> m_suspendedSets;

};

#endif // COMMANDBUFFERVK_H
