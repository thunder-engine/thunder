#include "resources/materialvk.h"

#include "commandbuffervk.h"

#include "resources/text.h"
#include "resources/texturevk.h"

#include "rendervksystem.h"

#include <file.h>
#include <log.h>

#define VERTEX_BIND     0
#define FRAGMENT_BIND   1
#define CAMERA_BIND     2
#define LIGHT_BIND      3
#define SAMPLE_BIND     4

void MaterialVk::loadUserData(const VariantMap &data) {
    Material::loadUserData(data);

    if(m_MaterialType == Surface) {
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

bool MaterialVk::getProgram(uint16_t type, VkPipeline &pipeline, VkPipelineLayout &layout) {
    switch(state()) {
        case Suspend: {
            destroyPrograms();

            setState(ToBeDeleted);
        } break;
        case ToBeUpdated: {
            destroyPrograms();

            for(uint16_t v = Static; v < LastVertex; v++) {
                auto itv = m_shaderSources.find(v);
                if(itv != m_shaderSources.end()) {
                    for(uint16_t f = Default; f < LastFragment; f++) {
                        auto itf = m_shaderSources.find(f);
                        if(itf != m_shaderSources.end()) {
                            VkShaderModule vertex = buildShader(itv->second);
                            VkShaderModule fragment = buildShader(itf->second);

                            buildStage(vertex, fragment, v * f);
                        }
                    }
                }
            }

            setState(Ready);
        } break;
        default: break;
    }

    auto it = m_programs.find(type);
    if(it != m_programs.end()) {
        pipeline = it->second;
        layout = m_layouts[type];
        return true;
    }
    return false;
}

void MaterialVk::destroyPrograms() {
    VkDevice device = RenderVkSystem::currentDevice();

    for(auto &it : m_programs) {
        vkDestroyPipeline(device, it.second, nullptr);
    }
    m_programs.clear();

    for(auto &it : m_layouts) {
        vkDestroyPipelineLayout(device, it.second, nullptr);
    }
    m_layouts.clear();

    vkDestroyDescriptorSetLayout(device, m_uniformDescSetLayout, nullptr);
}

bool MaterialVk::bind(VkCommandBuffer buffer, VkPipelineLayout &layout, uint32_t layer, uint16_t vertex) {
    int32_t b = blendMode();

    if((layer & ICommandBuffer::DEFAULT || layer & ICommandBuffer::SHADOWCAST) &&
       (b == Material::Additive || b == Material::Translucent)) {
        return false;
    }
    if(layer & ICommandBuffer::TRANSLUCENT && b == Material::Opaque) {
        return false;
    }

    uint16_t type = MaterialVk::Default;
    if((layer & ICommandBuffer::RAYCAST) || (layer & ICommandBuffer::SHADOWCAST)) {
        type = MaterialVk::Simple;
    }

    VkPipeline pipeline;
    if(getProgram(vertex * type, pipeline, layout)) {
        vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        return true;
    }
    return false;
}

void MaterialVk::textureAttributes(int32_t index, VkImageView &imageView, VkSampler &sampler) {
    for(auto &it : m_Textures) {
        if(it.binding == index) {
            TextureVk *t = static_cast<TextureVk *>(it.texture);
            if(t) {
                t->attributes(imageView, sampler);
            }
        }
    }
}

string MaterialVk::textureName(int32_t index) {
    for(auto &it : m_Textures) {
        if(it.binding == index) {
            return it.name;
        }
    }
    return string();
}

int32_t MaterialVk::textureBinding(const string &name) {
    for(auto &it : m_Textures) {
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

bool MaterialVk::buildStage(VkShaderModule vertex, VkShaderModule fragment, uint32_t index) {
    const vector<VkVertexInputBindingDescription> vertexInputBindings = {
        { 0, sizeof(Vector3), VK_VERTEX_INPUT_RATE_VERTEX },
        { 1, sizeof(Vector2), VK_VERTEX_INPUT_RATE_VERTEX },
        { 2, sizeof(Vector3), VK_VERTEX_INPUT_RATE_VERTEX },
        { 3, sizeof(Vector3), VK_VERTEX_INPUT_RATE_VERTEX },
        { 4, sizeof(Vector4), VK_VERTEX_INPUT_RATE_VERTEX },
    };
    const vector<VkVertexInputAttributeDescription> vertexAttributes = {
        { VERTEX_ATRIB,  0, VK_FORMAT_R32G32B32_SFLOAT,    0 },
        { UV0_ATRIB,     1, VK_FORMAT_R32G32_SFLOAT,       0 },
        { NORMAL_ATRIB,  0, VK_FORMAT_R32G32B32_SFLOAT,    0 },
        { TANGENT_ATRIB, 0, VK_FORMAT_R32G32B32_SFLOAT,    0 },
        { COLOR_ATRIB,   0, VK_FORMAT_R32G32B32A32_SFLOAT, 0 },
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

    VkPipelineColorBlendStateCreateInfo colorBlendState = {};
    colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendState.attachmentCount = 1;
    colorBlendState.pAttachments = &defaultColorBlendAttachmentState;

    VkDevice device = RenderVkSystem::currentDevice();

    // Create descriptor set layout
    m_layoutBindings = {
        { VERTEX_BIND,    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT,   nullptr },
        { FRAGMENT_BIND,  VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
        { CAMERA_BIND,    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
        { LIGHT_BIND,     VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr }
    };

    m_layoutBindingsSizes = {
        sizeof(VertexBufferObject),
        sizeof(FragmentBufferObject),
        sizeof(CameraBufferObject),
        sizeof(LightBufferObject)
    };

    uint32_t binding = SAMPLE_BIND;
    for(auto &it : m_Textures) {
        if(it.binding > 0) {
            m_layoutBindings.push_back({ (uint32_t)it.binding, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                         1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr });

            /// \todo Doesn't takes into account case with textures skinned meshes in vertex shader

            binding = MAX((uint32_t)it.binding, binding);
            m_layoutBindingsSizes.push_back(0);
        }
    }

    if(!m_Uniforms.empty()) {
        binding++;
        m_layoutBindings.push_back({binding, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr});

        uint32_t size = 0;
        for(auto &it : m_Uniforms) {
            size += it.size;
        }
        m_layoutBindingsSizes.push_back(size);
    }

    m_uniformDescSetLayout = CommandBufferVk::createDescriptorSetLayout(m_layoutBindings);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &m_uniformDescSetLayout;

    VkPipelineLayout layout;
    if(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &layout) != VK_SUCCESS) {
        return false;
    }

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
    pipelineCreateInfo.layout = layout;
    pipelineCreateInfo.renderPass = RenderVkSystem::currentRenderPass();

    VkDynamicState dynEnable[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, /*VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY_EXT*/ };
    VkPipelineDynamicStateCreateInfo dyn = {};
    dyn.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dyn.dynamicStateCount = sizeof(dynEnable) / sizeof(VkDynamicState);
    dyn.pDynamicStates = dynEnable;
    pipelineCreateInfo.pDynamicState = &dyn;

    VkPipeline pipeline;
    if(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline) != VK_SUCCESS) {
        return false;
    }

    vkDestroyShaderModule(device, vertex, nullptr);
    vkDestroyShaderModule(device, fragment, nullptr);

    m_programs[index] = pipeline;
    m_layouts[index] = layout;

    return true;
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
    }
    return result;
}

MaterialInstanceVk::MaterialInstanceVk(Material *material) :
        MaterialInstance(material) {

}

void MaterialInstanceVk::createDescriptors(VkDescriptorSetLayout layout) {
    VkDevice device = RenderVkSystem::currentDevice();
    if(device == nullptr) {
        return;
    }
    size_t swapChainCount = RenderVkSystem::swapChainImageCount();

    MaterialVk *m = static_cast<MaterialVk *>(m_pMaterial);

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

    if(vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
        throw runtime_error("failed to create descriptor pool!");
    }

    m_uniformDescriptorSets.resize(swapChainCount);

    m_buffers.resize(swapChainCount);
    m_buffersMemory.resize(swapChainCount);
    m_bufferObjects.resize(swapChainCount);

    // Create descriptor sets
    vector<VkDescriptorSetLayout> layouts(swapChainCount, layout);
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = swapChainCount;
    allocInfo.pSetLayouts = layouts.data();

    if(vkAllocateDescriptorSets(device, &allocInfo, m_uniformDescriptorSets.data()) != VK_SUCCESS) {
        throw runtime_error("failed to allocate descriptor sets!");
    }

    for(size_t i = 0; i < swapChainCount; i++) {
        uint32_t count = m->layoutBindingsCount();
        for(uint32_t l = 0; l < count; l++) {
            VkDescriptorSetLayoutBinding binding = m->layoutBinding(l);

            VkWriteDescriptorSet descriptorWrite = {};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = m_uniformDescriptorSets[i];
            descriptorWrite.dstBinding = binding.binding;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = binding.descriptorType;
            descriptorWrite.descriptorCount = 1;

            void *dst = nullptr;
            if(binding.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                VkDeviceSize size = m->layoutBindingSize(l);

                VkBuffer buffer;
                VkDeviceMemory memory;
                CommandBufferVk::createBuffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                              buffer, memory);

                m_buffers[i].push_back(buffer);
                m_buffersMemory[i].push_back(memory);

                VkDescriptorBufferInfo bufferInfo = {};
                bufferInfo.buffer = buffer;
                bufferInfo.offset = 0;
                bufferInfo.range = size;

                descriptorWrite.pBufferInfo = &bufferInfo;

                vkMapMemory(device, memory, 0, size, 0, &dst);

                vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
                m_bufferObjects[i].push_back(dst);
            } else {
                VkDescriptorImageInfo imageInfo = {};
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                TextureVk *t = static_cast<TextureVk *>(texture(m->textureName(binding.binding).c_str()));
                if(t) {
                    t->attributes(imageInfo.imageView, imageInfo.sampler);
                } else {
                    m->textureAttributes(binding.binding, imageInfo.imageView, imageInfo.sampler);
                }

                descriptorWrite.pImageInfo = &imageInfo;

                vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
                m_bufferObjects[i].push_back(dst);
            }
        }
    }
}

bool MaterialInstanceVk::bind(const VertexBufferObject &vertex, const FragmentBufferObject &fragment, VkCommandBuffer buffer, uint32_t index, uint32_t layer) {
    MaterialVk *m = static_cast<MaterialVk *>(m_pMaterial);

    VkPipelineLayout layout;
    if(m->bind(buffer, layout, layer, surfaceType())) {
        if(m_uniformDescriptorSets.size() == 0) {
            createDescriptors(m->descriptorSetLayout());
        }
        *reinterpret_cast<VertexBufferObject *>(m_bufferObjects[index][0]) = vertex;
        *reinterpret_cast<FragmentBufferObject *>(m_bufferObjects[index][1]) = fragment;

        vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                layout, 0, 1, &m_uniformDescriptorSets[index], 0, nullptr);
        return true;
    }
    return false;
}

void MaterialInstanceVk::setTexture(const char *name, Texture *value, int32_t count) {
    MaterialInstance::setTexture(name, value, count);

    if(value && !m_uniformDescriptorSets.empty()) {
        VkDescriptorImageInfo imageInfo = {};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        TextureVk *t = static_cast<TextureVk *>(value);
        t->attributes(imageInfo.imageView, imageInfo.sampler);

        MaterialVk *m = static_cast<MaterialVk *>(m_pMaterial);

        for(int32_t i = 0; i < RenderVkSystem::swapChainImageCount(); i++) {
            VkWriteDescriptorSet descriptorWrite = {};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = m_uniformDescriptorSets[i];
            descriptorWrite.dstBinding = m->textureBinding(name);
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrite.descriptorCount = 1;

            descriptorWrite.pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(RenderVkSystem::currentDevice(), 1, &descriptorWrite, 0, nullptr);
        }
    }
}
