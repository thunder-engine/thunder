#ifndef COMPUTESHADERVK_H
#define COMPUTESHADERVK_H

#include <list>

#include <resources/computeshader.h>

#include <vulkan/vulkan.h>

class CommandBufferVk;

class ComputeInstanceVk : public ComputeInstance {
public:
    ComputeInstanceVk(ComputeShader *shader);
    ~ComputeInstanceVk() override;

    bool bind(CommandBufferVk &buffer);

    void destroyDescriptors();

private:
    void setTexture(const char *name, Texture *value) override;

    void createDescriptors(CommandBufferVk &buffer, VkDescriptorSetLayout layout);

private:
    VkDescriptorPool m_descriptorPool;

    VkDescriptorSet m_uniformDescriptorSet;

    std::vector<std::vector<VkBuffer>> m_buffers;
    std::vector<std::vector<VkDeviceMemory>> m_buffersMemory;

};

class ComputeShaderVk : public ComputeShader {
    A_OVERRIDE(ComputeShaderVk, ComputeShader, Resources)

    A_NOPROPERTIES()
    A_NOMETHODS()

public:
    ComputeShaderVk();

    void loadUserData(const VariantMap &data) override;

    void switchState(State state) override;

    bool bind(VkCommandBuffer buffer);

    VkPipelineLayout pipelineLayout() {
        if(m_pipelineLayout == nullptr) {
            buildPipelineLayout();
        }
        return m_pipelineLayout;
    }

    VkDescriptorSetLayout descriptorSetLayout() const { return m_uniformDescSetLayout; }

    VkDescriptorSetLayoutBinding &layoutBinding(uint32_t index) {
        return m_layoutBindings[index];
    }

    VkDeviceSize layoutBindingSize(uint32_t index) {
        return m_layoutBindingsSizes[index];
    }

    uint32_t layoutBindingsCount() const { return m_layoutBindings.size(); }

    void textureAttributes(int32_t index, VkDescriptorImageInfo &imageinfo);

    std::string textureName(int32_t index);
    int32_t textureBinding(const std::string &name);

    void removeInstance(ComputeInstanceVk *instance);

protected:
    VkPipeline getPipeline();

    void destroyPrograms();

    VkShaderModule buildShader(const ByteArray &src);

    void buildPipelineLayout();
    VkPipeline buildPipeline(VkDevice device);

    ComputeInstance *createInstance() override;

private:
    VkShaderModule m_shader;

    VkPipeline m_pipeline;

    ByteArray m_shaderSource;

    std::vector<VkDescriptorSetLayoutBinding> m_layoutBindings;

    std::vector<VkDeviceSize> m_layoutBindingsSizes;

    std::list<ComputeInstanceVk *> m_instances;

    VkPipelineLayout m_pipelineLayout;
    VkDescriptorSetLayout m_uniformDescSetLayout;

};

#endif // COMPUTESHADERVK_H
