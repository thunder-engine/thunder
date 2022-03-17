#ifndef MATERIALVK_H
#define MATERIALVK_H

#include <unordered_map>
#include <list>

#include <vulkan/vulkan.h>

#include <resources/material.h>

#include <commandbuffer.h>

class MaterialInstanceVk : public MaterialInstance {
public:
    MaterialInstanceVk(Material *material);

    void createDescriptors(VkDescriptorSetLayout layout);

    bool bind(const Global &global, const Local &local, VkCommandBuffer buffer, uint32_t index, uint32_t layer);

private:
    void setTexture(const char *name, Texture *value) override;

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

    typedef unordered_map<uint32_t, VkPipeline> PipelineMap;
    typedef unordered_map<uint32_t, VkPipelineLayout> LayoutMap;

public:
    void loadUserData(const VariantMap &data) override;

    bool bind(VkCommandBuffer buffer, VkPipelineLayout &layout, uint32_t layer, uint16_t vertex);

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

protected:
    bool getProgram(uint16_t type, VkPipeline &pipeline, VkPipelineLayout &layout);

    void destroyPrograms();

    VkShaderModule buildShader(const ByteArray &src);

    bool buildStage(VkShaderModule vertex, VkShaderModule fragment, uint32_t index);

    MaterialInstance *createInstance(SurfaceType type = SurfaceType::Static) override;

private:
    PipelineMap m_programs;
    LayoutMap m_layouts;

    VkDescriptorSetLayout m_uniformDescSetLayout;

    map<uint16_t, ByteArray> m_shaderSources;

    vector<VkDescriptorSetLayoutBinding> m_layoutBindings;

    vector<VkDeviceSize> m_layoutBindingsSizes;

};

#endif // MATERIALVK_H
