#include "resources/computeshadervk.h"

#include <cstring>

#include "commandbuffervk.h"
#include "wrappervk.h"

#include "resources/texturevk.h"
#include "resources/computebuffervk.h"

#include <log.h>

ComputeShaderVk::ComputeShaderVk() :
        m_shader(VK_NULL_HANDLE),
        m_pipeline(VK_NULL_HANDLE),
        m_pipelineLayout(VK_NULL_HANDLE),
        m_uniformDescSetLayout(VK_NULL_HANDLE) {

}

void ComputeShaderVk::loadUserData(const VariantMap &data) {
    ComputeShader::loadUserData(data);

    auto it = data.find("Shader");
    if(it != data.end()) {
        m_shaderSource = (*it).second.toByteArray();
    }

    setState(ToBeUpdated);
}

void ComputeShaderVk::switchState(State state) {
    switch(state) {
        case Unloading: {
            destroyPrograms();
        } break;
        default: ComputeShader::switchState(state); break;
    }
}

VkPipeline ComputeShaderVk::getPipeline() {
    switch(state()) {
        case ToBeUpdated: {
            destroyPrograms();

            m_shader = buildShader(m_shaderSource);

            setState(Ready);
        } break;
        default: break;
    }

    if(m_pipeline == nullptr) {
        VkPipeline pipeline = buildPipeline(WrapperVk::device());
        if(pipeline) {
            m_pipeline = pipeline;
        }
    }
    return m_pipeline;
}

void ComputeShaderVk::destroyPrograms() {
    VkDevice device = WrapperVk::device();

    if(m_pipeline) {
        vkDestroyPipeline(device, m_pipeline, nullptr);
    }
    m_pipeline = nullptr;

    if(m_shader) {
        vkDestroyShaderModule(device, m_shader, nullptr);
    }
    m_shader = nullptr;

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

bool ComputeShaderVk::bind(VkCommandBuffer buffer) {
    VkPipeline pipeline = getPipeline();
    if(pipeline) {
        vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);

        return true;
    }
    return false;
}

void ComputeShaderVk::textureAttributes(int32_t index, VkDescriptorImageInfo &imageinfo) {
    for(auto &it : m_textures) {
        if(it.binding == index) {
            TextureVk *t = static_cast<TextureVk *>(it.texture);
            if(t) {
                t->attributes(imageinfo);
            }
        }
    }
}

TString ComputeShaderVk::textureName(int32_t index) {
    for(auto &it : m_textures) {
        if(it.binding == index) {
            return it.name;
        }
    }
    return std::string();
}

int32_t ComputeShaderVk::textureBinding(const TString &name) {
    for(auto &it : m_textures) {
        if(it.name == name) {
            return it.binding;
        }
    }
    return -1;
}

VkShaderModule ComputeShaderVk::buildShader(const ByteArray &src) {
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

void ComputeShaderVk::buildPipelineLayout() {
    if(m_pipelineLayout == nullptr && m_uniformDescSetLayout == nullptr) {
        // Create descriptor set layout
        m_layoutBindings = {
            { GLOBAL_BIND,  VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr }
        };

        m_layoutBindingsSizes = {
            sizeof(Global)
        };

        if(!m_uniforms.empty()) {
            m_layoutBindings.push_back({UNIFORM_BIND, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr});

            m_layoutBindingsSizes.push_back(m_uniformSize);
        }

        for(auto &it : m_textures) {
            if(it.binding > 0) {
                m_layoutBindings.push_back({ (uint32_t)it.binding, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                             1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr });

                m_layoutBindingsSizes.push_back(0);
            }
        }

        for(auto &it : m_buffers) {
            if(it.binding > 0) {
                m_layoutBindings.push_back({ (uint32_t)it.binding, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                             1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr });

                m_layoutBindingsSizes.push_back(0);
            }
        }


        m_uniformDescSetLayout = WrapperVk::createDescriptorSetLayout(m_layoutBindings);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &m_uniformDescSetLayout;

        if(vkCreatePipelineLayout(WrapperVk::device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
            aWarning() << "Unable to create plipeline layout.";
        }
    }
}

VkPipeline ComputeShaderVk::buildPipeline(VkDevice device) {
    if(m_shader == nullptr) {
        return nullptr;
    }

    VkPipelineShaderStageCreateInfo shaderStageInfo = {};
    shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    shaderStageInfo.module = m_shader;
    shaderStageInfo.pName = "main";

    VkComputePipelineCreateInfo pipelineCreateInfo = {};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.flags = 0;
    pipelineCreateInfo.stage = shaderStageInfo;
    pipelineCreateInfo.layout = pipelineLayout();
    pipelineCreateInfo.basePipelineIndex = -1;

    VkPipeline pipeline;
    if(vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline) == VK_SUCCESS) {
        return pipeline;
    }

    return nullptr;
}

ComputeInstance *ComputeShaderVk::createInstance() {
    ComputeInstanceVk *result = new ComputeInstanceVk(this);

    initInstance(result);

    if(result) {
        m_instances.push_back(result);
    }
    return result;
}

void ComputeShaderVk::removeInstance(ComputeInstanceVk *instance) {
    m_instances.remove(instance);
}

ComputeInstanceVk::ComputeInstanceVk(ComputeShader *shader) :
        ComputeInstance(shader),
        m_descriptorPool(nullptr),
        m_uniformDescriptorSet(nullptr) {

}

ComputeInstanceVk::~ComputeInstanceVk() {
    ComputeShaderVk *m = static_cast<ComputeShaderVk *>(m_compute);
    m->removeInstance(this);

    destroyDescriptors();
}

void ComputeInstanceVk::createDescriptors(CommandBufferVk &buffer, VkDescriptorSetLayout layout) {
    VkDevice device = WrapperVk::device();

    size_t swapChainCount = 1;//WrapperVk::swapChainImageCount();

    ComputeShaderVk *m = static_cast<ComputeShaderVk *>(m_compute);

    // Descriptor pool
    std::vector<VkDescriptorPoolSize> poolSize;
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
        throw std::runtime_error("failed to create descriptor pool!");
    }

    m_buffers.resize(swapChainCount);
    m_buffersMemory.resize(swapChainCount);

    // Create descriptor sets
    std::vector<VkDescriptorSetLayout> layouts(swapChainCount, layout);
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = layouts.data();

    if(vkAllocateDescriptorSets(device, &allocInfo, &m_uniformDescriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    uint32_t count = m->layoutBindingsCount();
    for(int32_t i = 0; i < swapChainCount; i++) {
        for(uint32_t l = 0; l < count; l++) {
            VkDescriptorSetLayoutBinding binding = m->layoutBinding(l);

            VkWriteDescriptorSet descriptorWrite = {};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstBinding = binding.binding;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = binding.descriptorType;
            descriptorWrite.descriptorCount = 1;

            if(binding.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
/*
                descriptorWrite.dstSet = buffer.globalDescriptorSet();

                VkDeviceSize size = m->layoutBindingSize(l);

                VkBuffer buffer;
                VkDeviceMemory memory;
                WrapperVk::createBuffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, buffer);
                WrapperVk::allocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer, memory);

                VkDescriptorBufferInfo bufferInfo = {};
                bufferInfo.buffer = buffer;
                bufferInfo.offset = 0;
                bufferInfo.range = size;

                descriptorWrite.pBufferInfo = &bufferInfo;

                m_buffers[i].push_back(buffer);
                m_buffersMemory[i].push_back(memory);

                vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
*/
            } else {
                descriptorWrite.dstSet = m_uniformDescriptorSet;

                TString name = m->textureName(binding.binding);
                TextureVk *t = static_cast<TextureVk *>(texture(name));
                if(t == nullptr) {
                    t = static_cast<TextureVk *>(buffer.texture(name));
                }

                VkDescriptorImageInfo imageInfo = {};
                if(t) {
                    t->attributes(imageInfo);
                } else {
                    m->textureAttributes(binding.binding, imageInfo);
                    if(imageInfo.imageView == nullptr || imageInfo.sampler == nullptr) {
                        static TextureVk *invalid = nullptr;
                        if(invalid == nullptr) {
                            invalid = static_cast<TextureVk *>(Engine::loadResource<Texture>(".embedded/invalid.png"));
                        }
                        invalid->attributes(imageInfo);
                    }
                }

                descriptorWrite.pImageInfo = &imageInfo;

                vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
            }
        }
    }
}

void ComputeInstanceVk::destroyDescriptors() {
    VkDevice device = WrapperVk::device();

    size_t swapChainCount = 1;//RenderVkSystem::swapChainImageCount();
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

bool ComputeInstanceVk::bind(CommandBufferVk &buffer) {
    ComputeShaderVk *m = static_cast<ComputeShaderVk *>(m_compute);

    VkCommandBuffer cmd = buffer.nativeBuffer();
    if(m->bind(cmd)) {
        if(m_buffers.size() == 0) {
            createDescriptors(buffer, m->descriptorSetLayout());
        }

        std::vector<const void *> buffers;

        if(m_uniformBuffer != nullptr && m_uniformDirty) {
            buffers.push_back(m_uniformBuffer);
            m_uniformDirty = false;
        }

        VkDevice device = WrapperVk::device();

        int32_t swapChainCount = 1;//RenderVkSystem::swapChainImageCount();
        for(int32_t index = 0; index < swapChainCount; index++) {
            for(int i = 0; i < buffers.size(); i++) {
                VkDeviceSize size = m->layoutBindingSize(i);

                void *dst = nullptr;
                vkMapMemory(device, m_buffersMemory[index][i], 0, size, 0, &dst);
                    memcpy(dst, buffers[i], size);
                vkUnmapMemory(device, m_buffersMemory[index][i]);
            }
        }

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE,
                            m->pipelineLayout(), 0, 1, &m_uniformDescriptorSet, 0, nullptr);

        return true;
    }
    return false;
}

void ComputeInstanceVk::setTexture(const TString &name, Texture *value) {
    if(m_textureOverride[name] != value) {
        ComputeInstance::setTexture(name, value);

        destroyDescriptors();
    }
}
