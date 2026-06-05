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

    bool bind(CommandBufferVk &buffer, uint32_t layer, VkDescriptorSet globalDescriptorSet, uint32_t currentFrame);

    void destroyDescriptors();

private:
    void overrideTexture(int32_t binding, Texture *texture) override;

    void updateDescriptors(const std::vector<VkDescriptorSetLayoutBinding> &bindings, uint32_t currentFrame);

    void updateTextures(const std::vector<VkDescriptorSetLayoutBinding> &bindings, CommandBufferVk &cmd, uint32_t currentFrame);

    static void textureUpdated(int state, void *object);

private:
    VkDescriptorPool m_descriptorPool;
    VkDeviceSize m_localSize;

    struct LocalResources {
        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;

        bool dirtyInstance = true;

        bool dirtyTextures = true;
    };

    std::vector<LocalResources> m_local;
};

class MaterialVk : public Material {
    A_OBJECT_OVERRIDE(MaterialVk, Material, Resources)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

    enum ShaderType {
        VertexStatic      = 1,
        VertexSkinned,
        VertexParticle,
        VertexLast,

        FragmentDefault,
        FragmentVisibility,
        FragmentLast
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

    VkDescriptorSetLayout localDescriptorSetLayout() const;

    const std::vector<VkDescriptorSetLayoutBinding> &localLayoutBindings() { return m_localLayoutBindings; }

    void removeInstance(MaterialInstanceVk *instance);

protected:
    VkPipeline getPipeline(uint16_t vertex, uint32_t layer, RenderTargetVk *target);

    void destroyPrograms();

    VkShaderModule buildShader(const ByteArray &src);

    void buildPipelineLayout();
    VkPipeline buildPipeline(uint32_t vertex, uint32_t layer, RenderTargetVk *target);

    MaterialInstance *createInstance(SurfaceType type = SurfaceType::Static) override;

private:
    std::unordered_map<uint32_t, VkShaderModule> m_shaders;

    std::unordered_map<uint32_t, std::vector<Attribute>> m_attributes;

    std::unordered_map<uint32_t, VkPipeline> m_pipelines;

    std::map<uint16_t, ByteArray> m_shaderSources;

    std::vector<VkDescriptorSetLayoutBinding> m_localLayoutBindings;

    std::list<MaterialInstanceVk *> m_instances;

    VkPipelineLayout m_pipelineLayout;

    VkDescriptorSetLayout m_localDescSetLayout;

};

#endif // MATERIALVK_H
