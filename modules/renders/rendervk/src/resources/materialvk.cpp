#include "resources/materialvk.h"

#include "resources/texturevk.h"
#include "resources/rendertargetvk.h"

#include "commandbuffervk.h"
#include "wrappervk.h"

#include <log.h>

namespace  {
    const char *gVisibility("Visibility");
    const char *gDefault("Default");

    const char *gStatic("Static");
    const char *gStaticInst("StaticInst");
    const char *gSkinned("Skinned");
    const char *gParticle("Particle");
    const char *gFullscreen("Fullscreen");
};

MaterialVk::MaterialVk() :
        m_pipelineLayout(nullptr),
        m_localDescSetLayout(nullptr) {

}

void MaterialVk::loadUserData(const VariantMap &data) {
    Material::loadUserData(data);

    static map<string, uint32_t> pairs = {
        {gVisibility, FragmentVisibility},
        {gDefault, FragmentDefault},

        {gStatic, VertexStatic},
        {gSkinned, VertexSkinned},
        {gParticle, VertexParticle}
    };

    for(auto &pair : pairs) {
        auto it = data.find(pair.first);
        if(it != data.end()) {
            m_shaderSources[pair.second] = (*it).second.toByteArray();
        }
    }

    setState(ToBeUpdated);
}

void MaterialVk::switchState(State state) {
    switch(state) {
        case Unloading: {
            destroyPrograms();
        } break;
        default: Material::switchState(state); break;
    }
}

VkPipeline MaterialVk::getPipeline(uint16_t vertex, uint16_t fragment, RenderTargetVk *target) {
    switch(state()) {
        case ToBeUpdated: {
            destroyPrograms();

            for(uint16_t v = VertexStatic; v < VertexLast; v++) {
                auto it = m_shaderSources.find(v);
                if(it != m_shaderSources.end()) {
                    m_shaders[v] = buildShader(it->second);
                }
            }

            for(uint16_t f = FragmentDefault; f < FragmentLast; f++) {
                auto it = m_shaderSources.find(f);
                if(it != m_shaderSources.end()) {
                    m_shaders[f] = buildShader(it->second);
                }
            }

            setState(Ready);
        } break;
        default: break;
    }

    std::hash<RenderTargetVk *> hashTarget;

    uint32_t index = (vertex | (fragment << 16)) ^ (hashTarget(target) << 2);

    auto it = m_pipelines.find(index);
    if(it != m_pipelines.end()) {
        return it->second;
    } else {
        VkPipeline pipeline = buildPipeline(vertex, fragment, WrapperVk::device(), target);
        if(pipeline) {
            m_pipelines[index] = pipeline;
            return pipeline;
        }
    }
    return nullptr;
}

void MaterialVk::destroyPrograms() {
    VkDevice device = WrapperVk::device();

    for(auto &it : m_pipelines) {
        vkDestroyPipeline(device, it.second, nullptr);
    }
    m_pipelines.clear();

    for(auto &it : m_shaders) {
        vkDestroyShaderModule(device, it.second, nullptr);
    }
    m_shaders.clear();

    if(m_pipelineLayout) {
        vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);
        m_pipelineLayout = VK_NULL_HANDLE;
    }

    if(m_localDescSetLayout) {
        vkDestroyDescriptorSetLayout(device, m_localDescSetLayout, nullptr);
        m_pipelineLayout = VK_NULL_HANDLE;
    }

    for(auto it : m_instances) {
        it->destroyDescriptors();
    }
}

VkPipelineLayout MaterialVk::pipelineLayout() {
    if(m_pipelineLayout == nullptr) {
        buildPipelineLayout();
    }
    return m_pipelineLayout;
}

VkDescriptorSetLayout MaterialVk::descriptorSetLayout() const {
    return m_localDescSetLayout;
}

bool MaterialVk::bind(VkCommandBuffer buffer, RenderTargetVk *target, uint32_t layer, uint16_t vertex) {
    if((layer & CommandBuffer::DEFAULT || layer & CommandBuffer::SHADOWCAST) && m_blendState.enabled) {
        return false;
    }
    if(layer & CommandBuffer::TRANSLUCENT && !m_blendState.enabled) {
        return false;
    }

    uint16_t type = FragmentDefault;
    if((layer & CommandBuffer::RAYCAST) || (layer & CommandBuffer::SHADOWCAST)) {
        type = FragmentVisibility;
    }

    VkPipeline pipeline = getPipeline(vertex, type, target);
    if(pipeline) {
        vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        return true;
    }
    return false;
}

TextureVk * MaterialVk::texture(int32_t index) {
    for(auto &it : m_textures) {
        if(it.binding == index) {
            return static_cast<TextureVk *>(it.texture);
        }
    }

    return nullptr;
}

string MaterialVk::textureName(int32_t index) {
    for(auto &it : m_textures) {
        if(it.binding == index) {
            return it.name;
        }
    }
    return string();
}

int32_t MaterialVk::textureBinding(const string &name) {
    for(auto &it : m_textures) {
        if(it.name == name) {
            return it.binding;
        }
    }
    return -1;
}

VkShaderModule MaterialVk::buildShader(const ByteArray &src) {
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = src.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(src.data());

    VkShaderModule shaderModule;
    if(vkCreateShaderModule(WrapperVk::device(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        return nullptr;
    }
    return shaderModule;
}

void MaterialVk::buildPipelineLayout() {
    if(m_pipelineLayout == VK_NULL_HANDLE && m_localDescSetLayout == VK_NULL_HANDLE) {
        // Create descriptor set layout
        m_layoutBindings = {{ LOCAL_BIND,
                              (materialType() == Surface) ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                              1,
                              VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                              nullptr }};

        for(auto &it : m_textures) {
            if(it.binding > 0) {
                m_layoutBindings.push_back({ (uint32_t)it.binding,
                                             VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                             1,
                                             VK_SHADER_STAGE_FRAGMENT_BIT,
                                             nullptr });
            }
        }

        m_localDescSetLayout = WrapperVk::createDescriptorSetLayout(m_layoutBindings);
        m_pipelineLayout = WrapperVk::createPipelineLayout({m_localDescSetLayout, CommandBufferVk::globalDescriptorSetLayout()});
    }
}

VkPipeline MaterialVk::buildPipeline(uint32_t v, uint32_t f, VkDevice device, RenderTargetVk *target) {
    VkShaderModule vertex = m_shaders[v];
    VkShaderModule fragment = m_shaders[f];

    if(vertex == nullptr || fragment == nullptr) {
        return nullptr;
    }

    vector<VkVertexInputBindingDescription> vertexInputBindings;
    vector<VkVertexInputAttributeDescription> vertexAttributes;

    vertexInputBindings.resize(m_attributes.size());
    vertexAttributes.resize(m_attributes.size());

    for(uint32_t i = 0; i < m_attributes.size(); i++) {
        VkFormat format = VK_FORMAT_UNDEFINED;
        uint32_t size = 0;
        switch(m_attributes[i].format) {
            case MetaType::VECTOR2 : format = VK_FORMAT_R32G32_SFLOAT; size = sizeof(Vector2); break;
            case MetaType::VECTOR3 : format = VK_FORMAT_R32G32B32_SFLOAT; size = sizeof(Vector3); break;
            case MetaType::VECTOR4 : format = VK_FORMAT_R32G32B32A32_SFLOAT; size = sizeof(Vector4); break;
            default: break;
        }

        vertexInputBindings[i] = {i, size, VK_VERTEX_INPUT_RATE_VERTEX};
        vertexAttributes[i] = {m_attributes[i].location, i, format, 0};
    }

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertex;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragment;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo vertexInputState = {};
    vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    // Can be conditional?
    vertexInputState.vertexBindingDescriptionCount = (uint32_t)vertexInputBindings.size();
    vertexInputState.pVertexBindingDescriptions = vertexInputBindings.data();
    // Can be conditional?
    vertexInputState.vertexAttributeDescriptionCount = (uint32_t)vertexAttributes.size();
    vertexInputState.pVertexAttributeDescriptions = vertexAttributes.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
    inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyState.topology = m_wireframe ? VK_PRIMITIVE_TOPOLOGY_LINE_LIST : VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizationState = {};
    rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationState.cullMode = doubleSided() ? VK_CULL_MODE_NONE :
                                                  ((m_materialType == LightFunction) ? VK_CULL_MODE_FRONT_BIT : VK_CULL_MODE_BACK_BIT);
    rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizationState.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo defaultMultisampleState = {};
    defaultMultisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    defaultMultisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
    depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilState.depthTestEnable = m_depthState.enabled;
    depthStencilState.depthWriteEnable = m_depthState.writeEnabled;
    depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

    VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
    colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                               VK_COLOR_COMPONENT_G_BIT |
                                               VK_COLOR_COMPONENT_B_BIT |
                                               VK_COLOR_COMPONENT_A_BIT;

    BlendState blendState = m_blendState;
    if(f == FragmentVisibility) {
        blendState.sourceColorBlendMode = Material::One;
        blendState.sourceAlphaBlendMode = Material::One;

        blendState.destinationColorBlendMode = Material::Zero;
        blendState.destinationAlphaBlendMode = Material::Zero;
    }

    colorBlendAttachmentState.blendEnable = blendState.enabled;
    if(blendState.enabled) {
        colorBlendAttachmentState.srcColorBlendFactor = static_cast<VkBlendFactor>(blendState.sourceColorBlendMode);
        colorBlendAttachmentState.dstColorBlendFactor = static_cast<VkBlendFactor>(blendState.destinationColorBlendMode);

        colorBlendAttachmentState.srcAlphaBlendFactor = static_cast<VkBlendFactor>(blendState.sourceAlphaBlendMode);
        colorBlendAttachmentState.dstAlphaBlendFactor = static_cast<VkBlendFactor>(blendState.destinationAlphaBlendMode);

        colorBlendAttachmentState.colorBlendOp = static_cast<VkBlendOp>(blendState.colorOperation);
        colorBlendAttachmentState.alphaBlendOp = static_cast<VkBlendOp>(blendState.alphaOperation);
    }

    vector<VkPipelineColorBlendAttachmentState> blendStates;
    blendStates.resize(target->colorAttachmentCount(), colorBlendAttachmentState);

    VkPipelineColorBlendStateCreateInfo colorBlendState = {};
    colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendState.attachmentCount = blendStates.size();
    colorBlendState.pAttachments = blendStates.data();

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stageCount = 2;
    pipelineCreateInfo.pStages = shaderStages;
    pipelineCreateInfo.pVertexInputState = &vertexInputState;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pRasterizationState = &rasterizationState;
    pipelineCreateInfo.pMultisampleState = &defaultMultisampleState;
    pipelineCreateInfo.pDepthStencilState = &depthStencilState;
    pipelineCreateInfo.pColorBlendState = &colorBlendState;
    pipelineCreateInfo.layout = pipelineLayout();
    pipelineCreateInfo.renderPass = target->renderPass();

    VkDynamicState dynEnable[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dyn = {};
    dyn.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dyn.dynamicStateCount = sizeof(dynEnable) / sizeof(VkDynamicState);
    dyn.pDynamicStates = dynEnable;
    pipelineCreateInfo.pDynamicState = &dyn;

    VkPipeline pipeline;
    if(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline) == VK_SUCCESS) {
        return pipeline;
    }

    return nullptr;
}

MaterialInstance *MaterialVk::createInstance(SurfaceType type) {
    MaterialInstanceVk *result = new MaterialInstanceVk(this);

    initInstance(result);

    if(result) {
        uint16_t t = VertexStatic;
        switch(type) {
            case SurfaceType::Static: t = VertexStatic; break;
            case SurfaceType::Skinned: t = VertexSkinned; break;
            case SurfaceType::Billboard: t = VertexParticle; break;
            default: break;
        }

        result->setSurfaceType(t);

        m_instances.push_back(result);
    }
    return result;
}

void MaterialVk::removeInstance(MaterialInstanceVk *instance) {
    m_instances.remove(instance);
}

MaterialInstanceVk::MaterialInstanceVk(Material *material) :
        MaterialInstance(material),
        m_descriptorPool(VK_NULL_HANDLE),
        m_localDescriptorSet(VK_NULL_HANDLE),
        m_buffer(VK_NULL_HANDLE),
        m_bufferMemory(VK_NULL_HANDLE) {

}

MaterialInstanceVk::~MaterialInstanceVk() {
    MaterialVk *m = static_cast<MaterialVk *>(m_material);
    m->removeInstance(this);

    destroyDescriptors();
}

void MaterialInstanceVk::updateDescriptors(CommandBufferVk &buffer, ByteArray &gpu) {
    MaterialVk *materialVk = static_cast<MaterialVk *>(m_material);

    uint32_t layoutBindingsCount = materialVk->layoutBindingsCount();
    for(uint32_t layout = 0; layout < layoutBindingsCount; layout++) {
        VkDescriptorSetLayoutBinding binding = materialVk->layoutBindings()[layout];

        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_localDescriptorSet;
        descriptorWrite.dstBinding = binding.binding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = binding.descriptorType;
        descriptorWrite.descriptorCount = 1;

        if(binding.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER || binding.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
            VkBuffer buffer;
            VkDeviceMemory memory;

            VkBufferUsageFlags flags = (binding.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) ?
                                            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT : VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

            WrapperVk::createBuffer(gpu.size(), flags, buffer);
            WrapperVk::allocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer, memory);

            m_buffer = buffer;
            m_bufferMemory = memory;

            VkDescriptorBufferInfo bufferInfo = {};
            bufferInfo.buffer = buffer;
            bufferInfo.offset = 0;
            bufferInfo.range = VK_WHOLE_SIZE;

            descriptorWrite.pBufferInfo = &bufferInfo;
        } else {
            string name = materialVk->textureName(binding.binding);
            TextureVk *t = static_cast<TextureVk *>(texture(name.c_str()));
            if(t == nullptr) {
                t = materialVk->texture(binding.binding);
            }

            if(t == nullptr) {
                t = static_cast<TextureVk *>(buffer.texture(name.c_str()));

                if(t == nullptr) {
                    t = static_cast<TextureVk *>(Engine::loadResource<Texture>(".embedded/invalid.png"));
                }
            }

            VkDescriptorImageInfo imageInfo = {};
            if(t) {
                t->attributes(imageInfo);
            }

            descriptorWrite.pImageInfo = &imageInfo;
        }

        vkUpdateDescriptorSets(WrapperVk::device(), 1, &descriptorWrite, 0, nullptr);
    }
}

void MaterialInstanceVk::destroyDescriptors() {
    VkDevice device = WrapperVk::device();

    if(m_buffer) {
        vkDestroyBuffer(device, m_buffer, nullptr);
        m_buffer = VK_NULL_HANDLE;
    }

    if(m_bufferMemory) {
        vkFreeMemory(device, m_bufferMemory, nullptr);
        m_bufferMemory = VK_NULL_HANDLE;
    }

    if(m_localDescriptorSet) {
        vkFreeDescriptorSets(device, m_descriptorPool, 1, &m_localDescriptorSet);
        m_localDescriptorSet = VK_NULL_HANDLE;
    }

    if(m_descriptorPool) {
        vkDestroyDescriptorPool(device, m_descriptorPool, nullptr);
        m_descriptorPool = VK_NULL_HANDLE;
    }
}

bool MaterialInstanceVk::bind(CommandBufferVk &buffer, uint32_t layer) {
    MaterialVk *materialVk = static_cast<MaterialVk *>(m_material);

    VkCommandBuffer cmd = buffer.nativeBuffer();

    if(materialVk->bind(cmd, buffer.currentRenderTarget(), layer, surfaceType())) {
        ByteArray &gpuBuffer = m_batchBuffer.empty() ? rawUniformBuffer() : m_batchBuffer;

        VkDevice device = WrapperVk::device();

        if(m_buffer == VK_NULL_HANDLE) {
            size_t swapChainCount = 1;//RenderVkSystem::swapChainImageCount();

            MaterialVk *materialVk = static_cast<MaterialVk *>(m_material);

            // Descriptor pool
            vector<VkDescriptorPoolSize> poolSize;
            for(auto &binding : materialVk->layoutBindings()) {
                poolSize.push_back({ binding.descriptorType, (uint32_t)swapChainCount });
            }

            m_descriptorPool = WrapperVk::createDescriptorPool(poolSize, swapChainCount);
            m_localDescriptorSet = WrapperVk::createDescriptorSet(materialVk->descriptorSetLayout(), m_descriptorPool);

            updateDescriptors(buffer, gpuBuffer);
        }

        if(!gpuBuffer.empty()) {
            VkDeviceSize size = gpuBuffer.size();

            void *dst = nullptr;
            vkMapMemory(device, m_bufferMemory, 0, size, 0, &dst);
                memcpy(dst, gpuBuffer.data(), size);
            vkUnmapMemory(device, m_bufferMemory);
        }

        vector<VkDescriptorSet> sets = {
            m_localDescriptorSet,
            buffer.globalDescriptorSet()
        };

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, materialVk->pipelineLayout(), 0, sets.size(), sets.data(), 0, nullptr);

        return true;
    }
    return false;
}

void MaterialInstanceVk::setTexture(const char *name, Texture *value) {
    if(m_textureOverride[name] != value) {
        MaterialInstance::setTexture(name, value);

        destroyDescriptors();
    }
}
