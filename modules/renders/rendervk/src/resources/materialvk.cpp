#include "resources/materialvk.h"

#include <cstring>

#include "resources/texturevk.h"
#include "resources/rendertargetvk.h"

#include "commandbuffervk.h"
#include "wrappervk.h"

MaterialVk::MaterialVk() :
        m_pipelineLayout(VK_NULL_HANDLE),
        m_localDescSetLayout(VK_NULL_HANDLE) {

}

void MaterialVk::loadUserData(const VariantMap &data) {
    Material::loadUserData(data);

    static std::map<std::string, uint32_t> pairs = {
        {"Visibility", FragmentVisibility},
        {"Default", FragmentDefault},

        {"Static", VertexStatic},
        {"Skinned", VertexSkinned},
        {"Particle", VertexParticle}
    };

    m_attributes.clear();

    for(auto &pair : pairs) {
        auto it = data.find(pair.first);
        if(it != data.end()) {
            auto fields = (*it).second.toList();

            auto field = fields.begin(); // Shader data
            m_shaderSources[pair.second] = field->toByteArray();

            std::vector<Attribute> attributes;

            ++field; // Uniform locations
            ++field; // Attributes
            if(field != fields.end()) {
                for(auto &a : field->toList()) {
                    VariantList list = a.toList();

                    attributes.push_back({list.back().toInt(), static_cast<uint32_t>(list.front().toInt())});
                }

                m_attributes[pair.second] = attributes;
            }

            if(pair.second == FragmentVisibility) {
                m_layers |= Material::Visibility;
                if(m_layers & Opaque) {
                    //m_layers |= Material::Shadowcast;
                }
            }
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

    uint32_t index = target->uuid();
    Mathf::hashCombine(index, vertex);
    Mathf::hashCombine(index, fragment);

    auto it = m_pipelines.find(index);
    if(it != m_pipelines.end()) {
        return it->second;
    } else {
        VkPipeline pipeline = buildPipeline(vertex, fragment, target);
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
    }

    for(auto it : m_instances) {
        it->destroyDescriptors();
    }
}

VkPipelineLayout MaterialVk::pipelineLayout() {
    if(m_pipelineLayout == VK_NULL_HANDLE) {
        buildPipelineLayout();
    }
    return m_pipelineLayout;
}

VkDescriptorSetLayout MaterialVk::localDescriptorSetLayout() const {
    return m_localDescSetLayout;
}

bool MaterialVk::bind(VkCommandBuffer buffer, RenderTargetVk *target, uint32_t layer, uint16_t vertex) {
    uint16_t type = FragmentDefault;
    if((layer & Material::Visibility) || (layer & Material::Shadowcast)) {
        type = FragmentVisibility;
    }

    VkPipeline pipeline = getPipeline(vertex, type, target);
    if(pipeline) {
        vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        return true;
    }
    return false;
}

VkShaderModule MaterialVk::buildShader(const ByteArray &src) {
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = src.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(src.data());

    VkShaderModule shaderModule;
    if(vkCreateShaderModule(WrapperVk::device(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }

    return shaderModule;
}

void MaterialVk::buildPipelineLayout() {
    if(m_pipelineLayout == VK_NULL_HANDLE && m_localDescSetLayout == VK_NULL_HANDLE) {
        // Create local descriptor set layout
        m_localLayoutBindings = {{ LOCAL_BIND,
                              VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                              1,
                              VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                              nullptr }};

        for(auto &it : m_textures) {
            if(it.binding > 0) {
                m_localLayoutBindings.push_back({ (uint32_t)it.binding,
                                                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                1,
                                                VK_SHADER_STAGE_FRAGMENT_BIT,
                                                nullptr });
            }
        }

        // Create global descriptor set layout
        m_localDescSetLayout = WrapperVk::createDescriptorSetLayout(m_localLayoutBindings);

        m_pipelineLayout = WrapperVk::createPipelineLayout({m_localDescSetLayout, CommandBufferVk::globalDescriptorSetLayout()});
    }
}

VkPipeline MaterialVk::buildPipeline(uint32_t v, uint32_t f, RenderTargetVk *target) {
    VkShaderModule vertex = m_shaders[v];
    VkShaderModule fragment = m_shaders[f];

    if(vertex == nullptr || fragment == nullptr) {
        return VK_NULL_HANDLE;
    }

    std::vector<VkVertexInputBindingDescription> vertexInputBindings;
    std::vector<VkVertexInputAttributeDescription> vertexAttributes;

    std::vector<Attribute> &attributes = m_attributes[v];

    vertexInputBindings.resize(attributes.size());
    vertexAttributes.resize(attributes.size());

    for(uint32_t i = 0; i < attributes.size(); i++) {
        VkFormat format = VK_FORMAT_UNDEFINED;
        uint32_t size = 0;
        switch(attributes[i].format) {
            case MetaType::VECTOR2 : format = VK_FORMAT_R32G32_SFLOAT; size = sizeof(Vector2); break;
            case MetaType::VECTOR3 : format = VK_FORMAT_R32G32B32_SFLOAT; size = sizeof(Vector3); break;
            case MetaType::VECTOR4 : format = VK_FORMAT_R32G32B32A32_SFLOAT; size = sizeof(Vector4); break;
            default: break;
        }

        vertexInputBindings[i] = {i, size, VK_VERTEX_INPUT_RATE_VERTEX};
        vertexAttributes[i] = {static_cast<uint32_t>(attributes[i].location), i, format, 0};
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

    std::vector<VkPipelineColorBlendAttachmentState> blendStates;
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
    if(vkCreateGraphicsPipelines(WrapperVk::device(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline) == VK_SUCCESS) {
        return pipeline;
    }

    return VK_NULL_HANDLE;
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
        m_localSize(0) {

}

MaterialInstanceVk::~MaterialInstanceVk() {
    MaterialVk *m = static_cast<MaterialVk *>(m_material);
    m->removeInstance(this);

    destroyDescriptors();

    for(auto it : m_textureOverride) {
        if(it.second) {
            static_cast<TextureVk *>(it.second)->unsubscribe(this);
        }
    }
}

void MaterialInstanceVk::updateDescriptors(const std::vector<VkDescriptorSetLayoutBinding> &bindings, uint32_t currentFrame) {
    VkDevice device = WrapperVk::device();

    for(auto &binding : bindings) {
        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_local[currentFrame].descriptorSet;
        descriptorWrite.dstBinding = binding.binding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = binding.descriptorType;
        descriptorWrite.descriptorCount = 1;

        if(binding.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER ||
           binding.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {

            VkDescriptorBufferInfo bufferInfo = {};
            bufferInfo.buffer = m_local[currentFrame].buffer;
            bufferInfo.offset = 0;
            bufferInfo.range = VK_WHOLE_SIZE;

            descriptorWrite.pBufferInfo = &bufferInfo;

            vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
        }
    }

    m_local[currentFrame].dirtyInstance = false;
}

void MaterialInstanceVk::updateTextures(const std::vector<VkDescriptorSetLayoutBinding> &bindings, CommandBufferVk &cmd, uint32_t currentFrame) {
    VkDevice device = WrapperVk::device();

    for(auto &binding : bindings) {
        if(binding.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
            VkWriteDescriptorSet descriptorWrite = {};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = m_local[currentFrame].descriptorSet;
            descriptorWrite.dstBinding = binding.binding;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = binding.descriptorType;
            descriptorWrite.descriptorCount = 1;

            TextureVk *t = static_cast<TextureVk *>(texture(cmd, binding.binding));
            if(t) {
                VkDescriptorImageInfo imageInfo = {};
                t->attributes(imageInfo);
                descriptorWrite.pImageInfo = &imageInfo;

                vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
            }
        }
    }

    m_local[currentFrame].dirtyTextures = false;
}

void MaterialInstanceVk::destroyDescriptors() {
    VkDevice device = WrapperVk::device();

    for(uint32_t i = 0; i < m_local.size(); i++) {
        vkDestroyBuffer(device, m_local[i].buffer, nullptr);
        vkFreeMemory(device, m_local[i].memory, nullptr);
        vkFreeDescriptorSets(device, m_descriptorPool, 1, &m_local[i].descriptorSet);
    }
    m_local.clear();

    if(m_descriptorPool) {
        vkDestroyDescriptorPool(device, m_descriptorPool, nullptr);
        m_descriptorPool = VK_NULL_HANDLE;
    }
}

bool MaterialInstanceVk::bind(CommandBufferVk &buffer, uint32_t layer, VkDescriptorSet globalDescriptorSet, uint32_t currentFrame) {
    MaterialVk *materialVk = static_cast<MaterialVk *>(m_material);

    VkCommandBuffer cmd = buffer.nativeBuffer();

    if(materialVk->bind(cmd, static_cast<RenderTargetVk *>(buffer.renderTarget()), layer, surfaceType())) {
        size_t swapChainCount = WrapperVk::framesInFlight();
        if(m_descriptorPool == VK_NULL_HANDLE) {
            std::vector<VkDescriptorPoolSize> poolSize;
            for(auto &binding : materialVk->localLayoutBindings()) {
                poolSize.push_back({ binding.descriptorType, (uint32_t)swapChainCount });
            }
            m_descriptorPool = WrapperVk::createDescriptorPool(poolSize, swapChainCount);
        }

        const ByteArray &localBuffer = m_batchBuffer ? *m_batchBuffer : rawUniformBuffer();
        if(!localBuffer.empty()) {

            VkDeviceSize size = localBuffer.size();

            VkDevice device = WrapperVk::device();
            if(size > m_localSize || m_local.empty()) {
                m_localSize = size;

                for(uint32_t i = 0; i < m_local.size(); i++) {
                    buffer.suspendBuffer(m_local[i].buffer, m_local[i].memory);
                    m_local[i].buffer = VK_NULL_HANDLE;
                    m_local[i].memory = VK_NULL_HANDLE;
                }
                m_local.resize(swapChainCount);

                for(uint32_t i = 0; i < swapChainCount; i++) {
                    if(m_local[i].buffer == VK_NULL_HANDLE) {
                        m_local[i].buffer = WrapperVk::createBuffer(m_localSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
                        m_local[i].memory = WrapperVk::allocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_local[i].buffer);

                        m_local[i].dirtyInstance = true;
                    }

                    if(m_local[i].descriptorSet == VK_NULL_HANDLE) {
                        m_local[i].descriptorSet = WrapperVk::createDescriptorSet(materialVk->localDescriptorSetLayout(), m_descriptorPool);
                    }
                }
            }

            if(m_local[currentFrame].dirtyInstance) {
                updateDescriptors(materialVk->localLayoutBindings(), currentFrame);
            }

            if(m_local[currentFrame].dirtyTextures) {
                updateTextures(materialVk->localLayoutBindings(), buffer, currentFrame);
            }

            void *dst = nullptr;
            vkMapMemory(device, m_local[currentFrame].memory, 0, size, 0, &dst);
                memcpy(dst, localBuffer.data(), size);
            vkUnmapMemory(device, m_local[currentFrame].memory);
        }

        const std::vector<VkDescriptorSet> sets = {
            m_local[currentFrame].descriptorSet,
            globalDescriptorSet
        };

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, materialVk->pipelineLayout(), 0, sets.size(), sets.data(), 0, nullptr);

        return true;
    }
    return false;
}

void MaterialInstanceVk::textureUpdated(int state, void *object) {
    if(state == Resource::ToBeUpdated) {
        MaterialInstanceVk *instance = reinterpret_cast<MaterialInstanceVk *>(object);

        if(instance) {
            for(auto &it : instance->m_local) {
                it.dirtyTextures = true;
            }
        }
    }
}

void MaterialInstanceVk::overrideTexture(int32_t binding, Texture *texture) {
    auto it = m_textureOverride.find(binding);
    if(it != m_textureOverride.end()) {
        if(it->second) {
            static_cast<TextureVk *>(it->second)->unsubscribe(this);
        }
    }

    MaterialInstance::overrideTexture(binding, texture);

    if(texture) {
        static_cast<TextureVk *>(texture)->subscribe(&MaterialInstanceVk::textureUpdated, this);
    }
}
