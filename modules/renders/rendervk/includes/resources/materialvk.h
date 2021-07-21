#ifndef MATERIALVK_H
#define MATERIALVK_H

#include <unordered_map>
#include <list>

#include <vulkan/vulkan.h>

#include <resources/material.h>

#include <engine.h>

struct VertexBufferObject {
    Matrix4 m_Model;
    Matrix4 m_View;
    Matrix4 m_Projection;
};

struct FragmentBufferObject {
    Vector4 m_Color;
    float m_Clip;
    float m_Time;
};

struct CameraBufferObject {
    Matrix4 m_view;
    Matrix4 m_projection;
    Matrix4 m_projectionInv;
    Matrix4 m_screenToWorld;
    Matrix4 m_worldToScreen;
    Vector4 m_position;
    Vector4 m_target;
    Vector4 m_screen;
};

struct LightBufferObject {
    Matrix4 m_matrix[6];
    Vector4 m_tiles[6];
    Vector4 m_color;
    Vector4 m_lod;
    Vector4 m_map;
    Vector4 m_params; // x - brightness, y - radius/width, z - length/height, w - cutoff
    Vector4 m_shadows; // x - ambient, y - bias, z - shadows
    Vector3 m_position;
    Vector3 m_direction;
};

class MaterialInstanceVk : public MaterialInstance {
public:
    MaterialInstanceVk(Material *material);

    void createDescriptors(VkDescriptorSetLayout layout);

    bool bind(const VertexBufferObject &vertex, const FragmentBufferObject &fragment, VkCommandBuffer buffer, uint32_t index, uint32_t layer);

private:
    void setTexture(const char *name, Texture *value, int32_t count) override;

private:
    VkDescriptorPool m_descriptorPool;

    vector<VkDescriptorSet> m_uniformDescriptorSets;

    vector<vector<VkBuffer>> m_buffers;
    vector<vector<VkDeviceMemory>> m_buffersMemory;
    vector<vector<void *>> m_bufferObjects;

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
