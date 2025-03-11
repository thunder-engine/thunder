#ifndef MATERIALVK_H
#define MATERIALVK_H

#include <unordered_map>
#include <list>

#include <vulkan/vulkan.h>

#include <resources/material.h>

struct Global;
class CommandBufferVk;

class RenderTargetVk;
class TextureVk;

class MaterialInstanceVk : public MaterialInstance {
public:
    MaterialInstanceVk(Material *material);
    ~MaterialInstanceVk() override;

    bool bind(CommandBufferVk &buffer, uint32_t layer, const Global &global);

    void destroyDescriptors();

private:
    void overrideTexture(int32_t binding, Texture *texture) override;

    void updateDescriptors(const std::vector<VkDescriptorSetLayoutBinding> &bindings, CommandBufferVk &cmd, VkDescriptorSet set, VkBuffer &buffer, VkDeviceMemory &memory, VkDeviceSize size);

    static void textureUpdated(int state, void *object);

private:
    VkDescriptorPool m_descriptorPool;

    VkDescriptorSet m_globalDescriptorSet;
    VkDescriptorSet m_localDescriptorSet;
    VkDescriptorSet m_suspendDescriptorSet;

    VkBuffer m_globalBuffer;
    VkBuffer m_localBuffer;

    VkDeviceMemory m_globalMemory;
    VkDeviceMemory m_localMemory;
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

    struct Attribute {
        int32_t location;

        uint32_t format;
    };

public:
    MaterialVk();

    void loadUserData(const VariantMap &data) override;

    void switchState(State state) override;

    bool bind(VkCommandBuffer buffer, RenderTargetVk *target, uint32_t layer, uint16_t vertex);

    VkPipelineLayout pipelineLayout();

    VkDescriptorSetLayout globalDescriptorSetLayout() const;
    VkDescriptorSetLayout localDescriptorSetLayout() const;

    const std::vector<VkDescriptorSetLayoutBinding> &globalLayoutBindings() { return m_globalLayoutBindings; }
    const std::vector<VkDescriptorSetLayoutBinding> &localLayoutBindings() { return m_localLayoutBindings; }

    void removeInstance(MaterialInstanceVk *instance);

protected:
    VkPipeline getPipeline(uint16_t vertex, uint16_t fragment, RenderTargetVk *target);

    void destroyPrograms();

    VkShaderModule buildShader(const ByteArray &src);

    void buildPipelineLayout();
    VkPipeline buildPipeline(uint32_t v, uint32_t f, RenderTargetVk *target);

    MaterialInstance *createInstance(SurfaceType type = SurfaceType::Static) override;

private:
    std::unordered_map<uint32_t, VkShaderModule> m_shaders;

    std::unordered_map<uint32_t, std::vector<Attribute>> m_attributes;

    std::unordered_map<uint32_t, VkPipeline> m_pipelines;

    std::map<uint16_t, ByteArray> m_shaderSources;

    std::vector<VkDescriptorSetLayoutBinding> m_globalLayoutBindings;
    std::vector<VkDescriptorSetLayoutBinding> m_localLayoutBindings;

    VkPipelineLayout m_pipelineLayout;

    VkDescriptorSetLayout m_globalDescSetLayout;
    VkDescriptorSetLayout m_localDescSetLayout;

    std::list<MaterialInstanceVk *> m_instances;

};

#endif // MATERIALVK_H
