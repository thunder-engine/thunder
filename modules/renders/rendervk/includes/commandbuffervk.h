/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
#ifndef COMMANDBUFFERVK_H
#define COMMANDBUFFERVK_H

#include <commandbuffer.h>

#include <vulkan/vulkan.h>

class RenderTargetVk;

class CommandBufferVk : public CommandBuffer {
    A_OBJECT_OVERRIDE(CommandBufferVk, CommandBuffer, System)

public:
    CommandBufferVk();
    ~CommandBufferVk();

    void begin(VkCommandBuffer buffer);

    void end();

    size_t currentFame() const;

    VkCommandBuffer nativeBuffer() const;

    void suspendBuffer(VkBuffer buffer, VkDeviceMemory memory);

    static std::vector<VkDescriptorSetLayoutBinding> &globalLayoutBindings();
    static VkDescriptorSetLayout globalDescriptorSetLayout();

private:
    void beginDebugMarker(const TString &name) override;
    void endDebugMarker() override;

    void dispatchCompute(ComputeInstance &shader, int32_t groupsX, int32_t groupsY, int32_t groupsZ) override;

    void drawMesh(Mesh *mesh, uint32_t sub, uint32_t layer, MaterialInstance &instance) override;

    void setRenderTarget(RenderTarget *target, uint32_t level = 0) override;

    void setViewport(int32_t x, int32_t y, int32_t width, int32_t height) override;

    void enableScissor(int32_t x, int32_t y, int32_t width, int32_t height) override;

    void disableScissor() override;

    void flipResult() override;

protected:
    VkCommandBuffer m_commandBuffer;

    VkViewport m_viewport;

    static PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT;
    static PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT;

    std::list<std::pair<VkBuffer, VkDeviceMemory>> m_suspended;

    size_t m_currentFrame;
};

#endif // COMMANDBUFFERVK_H
