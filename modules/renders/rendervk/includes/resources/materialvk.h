#ifndef MATERIALVK_H
#define MATERIALVK_H

#include <unordered_map>
#include <list>

#include <vulkan/vulkan.h>

#include <resources/material.h>

class CommandBufferVk;
struct Global;
struct Local;

class RenderTargetVk;

class MaterialInstanceVk : public MaterialInstance {
public:
    MaterialInstanceVk(Material *material);
    ~MaterialInstanceVk() override;

    bool bind(const Global &global, const Local &local, CommandBufferVk &buffer, uint32_t index, uint32_t layer);

    void destroyDescriptors();

private:
    void setTexture(const char *name, Texture *value) override;

    void createDescriptors(CommandBufferVk &buffer, VkDescriptorSetLayout layout);

private:
    VkDescriptorPool m_descriptorPool;

    VkDescriptorSet m_uniformDescriptorSet;

    vector<vector<VkBuffer>> m_buffers;
    vector<vector<VkDeviceMemory>> m_buffersMemory;

};

class MaterialVk : public Material {
    A_OVERRIDE(MaterialVk, Material, Resources)

    A_NOPROPERTIES()
    A_NOMETHODS()

    enum ShaderType {
        Static      = 1,
        Instanced,
        Skinned,
        Particle,
        LastVertex,

        Default     = 20,
        Simple,
        LastFragment
    };

public:
    MaterialVk();

    void loadUserData(const VariantMap &data) override;

    void switchState(ResourceState state) override;

    bool bind(VkCommandBuffer buffer, RenderTargetVk *target, uint32_t layer, uint16_t vertex);

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

    void textureAttributes(int32_t index, VkImageView &imageView, VkSampler &sampler);

    string textureName(int32_t index);
    int32_t textureBinding(const string &name);

    void removeInstance(MaterialInstanceVk *instance);

protected:
    VkPipeline getPipeline(uint16_t vertex, uint16_t fragment, RenderTargetVk *target);

    void destroyPrograms();

    VkShaderModule buildShader(const ByteArray &src);

    void buildPipelineLayout();
    VkPipeline buildPipeline(uint32_t v, uint32_t f, RenderTargetVk *target);

    MaterialInstance *createInstance(SurfaceType type = SurfaceType::Static) override;

private:
    unordered_map<uint32_t, VkShaderModule> m_shaders;

    unordered_map<uint32_t, VkPipeline> m_pipelines;

    map<uint16_t, ByteArray> m_shaderSources;

    vector<VkDescriptorSetLayoutBinding> m_layoutBindings;

    vector<VkDeviceSize> m_layoutBindingsSizes;

    VkPipelineLayout m_pipelineLayout;
    VkDescriptorSetLayout m_uniformDescSetLayout;

    list<MaterialInstanceVk *> m_instances;

};

#endif // MATERIALVK_H
