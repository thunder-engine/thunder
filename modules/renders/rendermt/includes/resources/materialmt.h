#ifndef MATERIALMT_H
#define MATERIALMT_H

#include <unordered_map>
#include <list>

#include <resources/material.h>

#include <engine.h>

#include "wrappermt.h"

class CommandBufferMt;
class RenderTargetMt;

class Global;

class MaterialInstanceMt : public MaterialInstance {
public:
    MaterialInstanceMt(Material *material);

    ~MaterialInstanceMt();

    bool bind(CommandBufferMt &buffer, uint32_t layer, const Global &global);

private:
    MTL::Buffer *m_instanceBuffer;

    MTL::Buffer *m_globalBuffer;

    int32_t m_globalVertextLocation;
    int32_t m_localVertextLocation;

    int32_t m_globalFragmentLocation;
    int32_t m_localFragmentLocation;

};

class MaterialMt : public Material {
    A_OBJECT_OVERRIDE(MaterialMt, Material, Resources)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

    struct Attribute {
        int32_t location;

        uint32_t format;
    };

    struct Uniform {
        std::string name;

        int32_t location;
    };

    struct Shader {
        std::vector<Attribute> attributes;

        std::vector<Uniform> uniforms;

        MTL::Function *function;
    };

public:
    void loadUserData(const VariantMap &data) override;

    Shader *shader(uint16_t type);

    Textures &textures() { return m_textures; }

    bool bind(MTL::RenderCommandEncoder *encoder, RenderTargetMt *target, uint32_t layer, uint16_t vertex);

    MTL::DepthStencilState *depthStencilState() const { return m_depthStencilState; }

protected:
    MTL::Function *buildShader(const std::string &src) const;

    MTL::RenderPipelineState *getPipeline(uint16_t vertex, uint16_t fragment, RenderTargetMt *target);

    MTL::RenderPipelineState *buildPipeline(uint32_t v, uint32_t f, RenderTargetMt *target);

    MaterialInstance *createInstance(SurfaceType type = SurfaceType::Static) override;

private:
    friend class MaterialInstanceMt;

    std::unordered_map<uint16_t, Shader> m_pipelineFunctions;

    std::unordered_map<uint32_t, MTL::RenderPipelineState *> m_pipelines;

    MTL::DepthStencilState *m_depthStencilState;
};

#endif // MATERIALMT_H
