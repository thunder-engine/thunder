#include "resources/materialvk.h"

#include "commandbuffervk.h"

#include "resources/texturevk.h"
#include "resources/rendertargetvk.h"

#include "rendervksystem.h"

#include <log.h>

MaterialVk::MaterialVk() :
        m_pipelineLayout(nullptr),
        m_uniformDescSetLayout(nullptr) {

}

void MaterialVk::loadUserData(const VariantMap &data) {
    Material::loadUserData(data);

     if(m_materialType == Surface) {
        {
            auto it = data.find("Simple");
            if(it != data.end()) {
                m_shaderSources[Simple] = (*it).second.toByteArray();
            }
        }
        {
            auto it = data.find("StaticInst");
            if(it != data.end()) {
                m_shaderSources[Instanced] = (*it).second.toByteArray();
            }
        }
        {
            auto it = data.find("Particle");
            if(it != data.end()) {
                m_shaderSources[Particle] = (*it).second.toByteArray();
            }
        }
        {
            auto it = data.find("Skinned");
            if(it != data.end()) {
                m_shaderSources[Skinned] = (*it).second.toByteArray();
                setTexture("skinMatrices", nullptr);
            }
        }
    }

    {
        auto it = data.find("Shader");
        if(it != data.end()) {
            m_shaderSources[Default] = (*it).second.toByteArray();
        }
    }
    {
        auto it = data.find("Static");
        if(it != data.end()) {
            m_shaderSources[Static] = (*it).second.toByteArray();
        }
    }

    setState(ToBeUpdated);
}

void MaterialVk::switchState(ResourceState state) {
    switch(state) {
        case Unloading: {
            destroyPrograms();

            setState(ToBeDeleted);
        } break;
        default: Material::switchState(state); break;
    }
}

VkPipeline MaterialVk::getPipeline(uint16_t vertex, uint16_t fragment, RenderTargetVk *target) {
    switch(state()) {
        case ToBeUpdated: {
            destroyPrograms();

            for(uint16_t v = Static; v < LastVertex; v++) {
                auto it = m_shaderSources.find(v);
                if(it != m_shaderSources.end()) {
                    m_shaders[v] = buildShader(it->second);
                }
            }

            for(uint16_t f = Default; f < LastFragment; f++) {
                auto it = m_shaderSources.find(f);
                if(it != m_shaderSources.end()) {
                    m_shaders[f] = buildShader(it->second);
                }
            }

            setState(Ready);
        } break;
        default: break;
    }

    std::hash<RenderTargetVk *> hash;

    uint32_t index = (vertex | (fragment << 16)) ^ (hash(target) << 2);

    auto it = m_pipelines.find(index);
    if(it != m_pipelines.end()) {
        return it->second;
    } else {
        VkPipeline pipeline = buildPipeline(vertex, fragment, target);
        if(pipeline) {
            m_pipelines[index] = pipeline;
        }
    }
    return nullptr;
}

void MaterialVk::destroyPrograms() {
    VkDevice device = RenderVkSystem::currentDevice();

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
    }
    m_pipelineLayout = nullptr;

    if(m_uniformDescSetLayout) {
        vkDestroyDescriptorSetLayout(device, m_uniformDescSetLayout, nullptr);
    }
    m_uniformDescSetLayout = nullptr;

    for(auto it : m_instances) {
        it->destroyDescriptors();
    }
}

bool MaterialVk::bind(VkCommandBuffer buffer, RenderTargetVk *target, uint32_t layer, uint16_t vertex) {
    int32_t b = blendMode();

    if((layer & CommandBuffer::DEFAULT || layer & CommandBuffer::SHADOWCAST) &&
       (b == Material::Additive || b == Material::Translucent)) {
        return false;
    }
    if(layer & CommandBuffer::TRANSLUCENT && b == Material::Opaque) {
        return false;
    }

    uint16_t type = MaterialVk::Default;
    if((layer & CommandBuffer::RAYCAST) || (layer & CommandBuffer::SHADOWCAST)) {
        type = MaterialVk::Simple;
    }

    VkPipeline pipeline = getPipeline(vertex, type, target);
    if(pipeline) {
        vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        return true;
    }
    return false;
}

void MaterialVk::textureAttributes(int32_t index, VkImageView &imageView, VkSampler &sampler) {
    for(auto &it : m_textures) {
        if(it.binding == index) {
            TextureVk *t = static_cast<TextureVk *>(it.texture);
            if(t) {
                t->attributes(imageView, sampler);
            }
        }
    }
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
    if(vkCreateShaderModule(RenderVkSystem::currentDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        return nullptr;
    }
    return shaderModule;
}

void MaterialVk::buildPipelineLayout() {
    if(m_pipelineLayout == nullptr && m_uniformDescSetLayout == nullptr) {
        // Create descriptor set layout
        m_layoutBindings = {
            { GLOBAL_BIND,  VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
            { LOCAL_BIND,   VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
        };

        m_layoutBindingsSizes = {
            sizeof(Global),
            sizeof(Local)
        };

        if(!m_uniforms.empty()) {
            m_layoutBindings.push_back({UNIFORM_BIND, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr});

            m_layoutBindingsSizes.push_back(m_uniformSize);
        }

        for(auto &it : m_textures) {
            if(it.binding > 0) {
                m_layoutBindings.push_back({ (uint32_t)it.binding, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                             1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr });

                /// \todo Doesn't takes into account case with textures skinned meshes in vertex shader

                m_layoutBindingsSizes.push_back(0);
            }
        }

        m_uniformDescSetLayout = CommandBufferVk::createDescriptorSetLayout(m_layoutBindings);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &m_uniformDescSetLayout;

        if(vkCreatePipelineLayout(RenderVkSystem::currentDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
            aWarning() << "Unable to create plipeline layout.";
        }
    }
}

VkPipeline MaterialVk::buildPipeline(uint32_t v, uint32_t f, RenderTargetVk *target) {
    VkShaderModule vertex = m_shaders[v];
    VkShaderModule fragment = m_shaders[f];

    if(vertex == nullptr || fragment == nullptr) {
        return nullptr;
    }
    const vector<VkVertexInputBindingDescription> vertexInputBindings = {
        { 0, sizeof(Vector3), VK_VERTEX_INPUT_RATE_VERTEX },
        { 1, sizeof(Vector2), VK_VERTEX_INPUT_RATE_VERTEX },
        { 2, sizeof(Vector3), VK_VERTEX_INPUT_RATE_VERTEX },
        { 3, sizeof(Vector3), VK_VERTEX_INPUT_RATE_VERTEX },
        //{ 4, sizeof(Vector4), VK_VERTEX_INPUT_RATE_VERTEX },
    };
    const vector<VkVertexInputAttributeDescription> vertexAttributes = {
        { VERTEX_ATRIB,  0, VK_FORMAT_R32G32B32_SFLOAT,    0 },
        { UV0_ATRIB,     1, VK_FORMAT_R32G32_SFLOAT,       0 },
        { NORMAL_ATRIB,  0, VK_FORMAT_R32G32B32_SFLOAT,    0 },
        { TANGENT_ATRIB, 0, VK_FORMAT_R32G32B32_SFLOAT,    0 },
        //{ COLOR_ATRIB,   0, VK_FORMAT_R32G32B32A32_SFLOAT, 0 },
    };

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
    inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizationState = {};
    rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationState.cullMode = doubleSided() ? VK_CULL_MODE_NONE : VK_CULL_MODE_BACK_BIT;
    rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationState.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo defaultMultisampleState = {};
    defaultMultisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    defaultMultisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
    depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilState.depthTestEnable = depthTest() ? VK_TRUE : VK_FALSE;
    depthStencilState.depthWriteEnable = depthWrite() ? VK_TRUE : VK_FALSE;
    depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

    VkPipelineColorBlendAttachmentState defaultColorBlendAttachmentState = {};
    defaultColorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                                      VK_COLOR_COMPONENT_G_BIT |
                                                      VK_COLOR_COMPONENT_B_BIT |
                                                      VK_COLOR_COMPONENT_A_BIT;

    int32_t b = blendMode();
    if(b == Material::Opaque) {
        defaultColorBlendAttachmentState.blendEnable = VK_FALSE;
    } else {
        defaultColorBlendAttachmentState.blendEnable = VK_TRUE;

        if(b == Material::Translucent) {
            defaultColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            defaultColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

            defaultColorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            defaultColorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        } else {
            defaultColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            defaultColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;

            defaultColorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            defaultColorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        }

        defaultColorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
        defaultColorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
    }

    vector<VkPipelineColorBlendAttachmentState> blendStates;
    blendStates.resize(target->colorAttachmentCount(), defaultColorBlendAttachmentState);

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
    if(vkCreateGraphicsPipelines(RenderVkSystem::currentDevice(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline) == VK_SUCCESS) {
        return pipeline;
    }

    return nullptr;
}

MaterialInstance *MaterialVk::createInstance(SurfaceType type) {
    MaterialInstanceVk *result = new MaterialInstanceVk(this);

    initInstance(result);

    if(result) {
        uint16_t t = Instanced;
        switch(type) {
            case SurfaceType::Static: t = Static; break;
            case SurfaceType::Skinned: t = Skinned; break;
            case SurfaceType::Billboard: t = Particle; break;
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
        m_descriptorPool(nullptr),
        m_uniformDescriptorSet(nullptr) {

}

MaterialInstanceVk::~MaterialInstanceVk() {
    MaterialVk *m = static_cast<MaterialVk *>(m_material);
    m->removeInstance(this);

    destroyDescriptors();
}

void MaterialInstanceVk::createDescriptors(CommandBufferVk &buffer, VkDescriptorSetLayout layout) {
    VkDevice device = RenderVkSystem::currentDevice();
    if(device == nullptr) {
        return;
    }
    size_t swapChainCount = RenderVkSystem::swapChainImageCount();

    MaterialVk *m = static_cast<MaterialVk *>(m_material);

    // Descriptor pool
    vector<VkDescriptorPoolSize> poolSize;
    for(uint32_t i = 0; i < m->layoutBindingsCount(); i++) {
        VkDescriptorSetLayoutBinding &binding = m->layoutBinding(i);
        poolSize.push_back({binding.descriptorType, (uint32_t)swapChainCount});
    }

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = poolSize.size();
    poolInfo.pPoolSizes = poolSize.data();
    poolInfo.maxSets = swapChainCount;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    if(vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
        throw runtime_error("failed to create descriptor pool!");
    }

    m_buffers.resize(swapChainCount);
    m_buffersMemory.resize(swapChainCount);

    // Create descriptor sets
    vector<VkDescriptorSetLayout> layouts(swapChainCount, layout);
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = layouts.data();

    if(vkAllocateDescriptorSets(device, &allocInfo, &m_uniformDescriptorSet) != VK_SUCCESS) {
        throw runtime_error("failed to allocate descriptor sets!");
    }

    uint32_t count = m->layoutBindingsCount();
    for(int32_t i = 0; i < swapChainCount; i++) {
        for(uint32_t l = 0; l < count; l++) {
            VkDescriptorSetLayoutBinding binding = m->layoutBinding(l);

            VkWriteDescriptorSet descriptorWrite = {};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = m_uniformDescriptorSet;
            descriptorWrite.dstBinding = binding.binding;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = binding.descriptorType;
            descriptorWrite.descriptorCount = 1;

            if(binding.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                VkDeviceSize size = m->layoutBindingSize(l);

                VkBuffer buffer;
                VkDeviceMemory memory;
                CommandBufferVk::createBuffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                              buffer, memory);

                VkDescriptorBufferInfo bufferInfo = {};
                bufferInfo.buffer = buffer;
                bufferInfo.offset = 0;
                bufferInfo.range = size;

                descriptorWrite.pBufferInfo = &bufferInfo;

                m_buffers[i].push_back(buffer);
                m_buffersMemory[i].push_back(memory);

                vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
            } else {
                VkDescriptorImageInfo imageInfo = {};
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                string name = m->textureName(binding.binding);
                TextureVk *t = static_cast<TextureVk *>(texture(name.c_str()));
                if(t == nullptr) {
                    t = static_cast<TextureVk *>(buffer.texture(name.c_str()));
                }

                if(t) {
                    t->attributes(imageInfo.imageView, imageInfo.sampler);
                } else {
                    m->textureAttributes(binding.binding, imageInfo.imageView, imageInfo.sampler);
                    if(imageInfo.imageView == nullptr || imageInfo.sampler == nullptr) {
                        continue;
                    }
                }

                descriptorWrite.pImageInfo = &imageInfo;

                vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
            }
        }
    }
}

void MaterialInstanceVk::destroyDescriptors() {
    VkDevice device = RenderVkSystem::currentDevice();

    size_t swapChainCount = RenderVkSystem::swapChainImageCount();
    if(!m_buffers.empty()) {
        for(int32_t i = 0; i < swapChainCount; i++) {
            for(auto it : m_buffers[i]) {
                vkDestroyBuffer(device, it, nullptr);
            }
        }
        m_buffers.clear();
    }

    if(!m_buffersMemory.empty()) {
        for(int32_t i = 0; i < swapChainCount; i++) {
            for(auto it : m_buffersMemory[i]) {
                vkFreeMemory(device, it, nullptr);
            }
        }
        m_buffersMemory.clear();
    }

    if(m_uniformDescriptorSet) {
        vkFreeDescriptorSets(device, m_descriptorPool, 1, &m_uniformDescriptorSet);
        m_uniformDescriptorSet = nullptr;
    }

    if(m_descriptorPool) {
        vkDestroyDescriptorPool(device, m_descriptorPool, nullptr);
        m_descriptorPool = nullptr;
    }
}

bool MaterialInstanceVk::bind(const Global &global, const Local &local, CommandBufferVk &buffer, uint32_t index, uint32_t layer) {
    MaterialVk *m = static_cast<MaterialVk *>(m_material);

    VkCommandBuffer cmd = buffer.nativeBuffer();

    if(m->bind(cmd, buffer.currentRenderTarget(), layer, surfaceType())) {
        if(m_buffers.size() == 0) {
            createDescriptors(buffer, m->descriptorSetLayout());
        }

        vector<const void *> buffers = {
            &global,
            &local
        };

        if(m_uniformBuffer != nullptr && m_uniformDirty) {
            buffers.push_back(m_uniformBuffer);
            m_uniformDirty = false;
        }

        VkDevice device = RenderVkSystem::currentDevice();

        size_t swapChainCount = RenderVkSystem::swapChainImageCount();
        for(index = 0; index < swapChainCount; index++) {
            for(int i = 0; i < buffers.size(); i++) {
                VkDeviceSize size = m->layoutBindingSize(i);

                void *dst = nullptr;
                vkMapMemory(device, m_buffersMemory[index][i], 0, size, 0, &dst);
                    memcpy(dst, buffers[i], size);
                vkUnmapMemory(device, m_buffersMemory[index][i]);
            }
        }

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m->pipelineLayout(), 0, 1, &m_uniformDescriptorSet, 0, nullptr);

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
