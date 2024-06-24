#ifndef MATERIALVK_H
#define MATERIALVK_H

#include <unordered_map>
#include <list>

#include <vulkan/vulkan.h>

#include <resources/material.h>

class CommandBufferVk;

class RenderTargetVk;
class TextureVk;

class MaterialInstanceVk : public MaterialInstance {
public:
    MaterialInstanceVk(Material *material);
    ~MaterialInstanceVk() override;

    bool bind(CommandBufferVk &buffer, uint32_t layer);

    void destroyDescriptors();

private:
    void setTexture(const char *name, Texture *value) override;

    void updateDescriptors(CommandBufferVk &buffer, ByteArray &gpu);

private:
    VkDescriptorPool m_descriptorPool;

    VkDescriptorSet m_localDescriptorSet;

    VkBuffer m_buffer;
    VkDeviceMemory m_bufferMemory;

};

class MaterialVk : public Material {
    A_OVERRIDE(MaterialVk, Material, Resources)

    A_NOPROPERTIES()
    A_NOMETHODS()

    enum ShaderType {
        VertexStatic      = 1,
        VertexSkinned,
        VertexParticle,
        VertexLast,

        FragmentDefault,
        FragmentVisibility,
        FragmentLast,

        GeometryDefault,
        GeometryLast
    };

public:
    MaterialVk();

    void loadUserData(const VariantMap &data) override;

    void switchState(State state) override;

    bool bind(VkCommandBuffer buffer, RenderTargetVk *target, uint32_t layer, uint16_t vertex);

    VkPipelineLayout pipelineLayout();

    VkDescriptorSetLayout descriptorSetLayout() const;

    vector<VkDescriptorSetLayoutBinding> layoutBindings() { return m_layoutBindings; }

    uint32_t layoutBindingsCount() const { return m_layoutBindings.size(); }

    TextureVk *texture(int32_t index);

    string textureName(int32_t index);
    int32_t textureBinding(const string &name);

    void removeInstance(MaterialInstanceVk *instance);

protected:
    VkPipeline getPipeline(uint16_t vertex, uint16_t fragment, RenderTargetVk *target);

    void destroyPrograms();

    VkShaderModule buildShader(const ByteArray &src);

    void buildPipelineLayout();
    VkPipeline buildPipeline(uint32_t v, uint32_t f, VkDevice device, RenderTargetVk *target);

    MaterialInstance *createInstance(SurfaceType type = SurfaceType::Static) override;

private:
    unordered_map<uint32_t, VkShaderModule> m_shaders;

    unordered_map<uint32_t, VkPipeline> m_pipelines;

    map<uint16_t, ByteArray> m_shaderSources;

    vector<VkDescriptorSetLayoutBinding> m_layoutBindings;

    VkPipelineLayout m_pipelineLayout;
    VkDescriptorSetLayout m_localDescSetLayout;

    list<MaterialInstanceVk *> m_instances;

};

#endif // MATERIALVK_H
