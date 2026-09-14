/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
#ifndef MATERIALMT_H
#define MATERIALMT_H

#include <unordered_map>
#include <list>

#include <resources/material.h>

#include <engine.h>

#include "wrappermt.h"

class CommandBufferMt;
class RenderTargetMt;

class MaterialInstanceMt : public MaterialInstance {
public:
    MaterialInstanceMt(Material *material);

    ~MaterialInstanceMt();

    bool bind(CommandBufferMt &buffer, uint32_t layer, const MTL::Buffer *global, uint32_t currentFrame);

private:
    std::vector<MTL::Buffer *> m_local;

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
        TString name;

        int32_t location;
    };

    struct Shader {
        TString source;

        std::vector<Attribute> attributes;

        std::vector<Uniform> uniforms;

        MTL::Function *function;
    };

public:
    void loadUserData(const VariantMap &data) override;

    VariantMap saveUserData() const override;

    Shader *shader(uint16_t type);

    Textures &textures() { return m_textures; }

    bool bind(MTL::RenderCommandEncoder *encoder, RenderTargetMt *target, uint32_t layer, uint16_t vertex);

    MTL::DepthStencilState *depthStencilState() const { return m_depthStencilState; }

protected:
    MTL::Function *buildShader(const TString &src) const;

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
