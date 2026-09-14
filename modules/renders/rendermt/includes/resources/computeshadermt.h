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
#ifndef COMPUTESHADERMT_H
#define COMPUTESHADERMT_H

#include <unordered_map>
#include <list>

#include <resources/computeshader.h>

#include <engine.h>

#include "wrappermt.h"

class CommandBufferMt;

class ComputeInstanceMt : public ComputeInstance {
public:
    ComputeInstanceMt(ComputeShader *compute);

    ~ComputeInstanceMt();

    bool bind(CommandBufferMt *buffer, MTL::ComputeCommandEncoder *encoder);

    int32_t maxTotalThreadsPerThreadgroup() const;

private:
    MTL::Buffer *m_data;

};

class ComputeShaderMt : public ComputeShader {
    A_OBJECT_OVERRIDE(ComputeShaderMt, ComputeShader, Resources)

    A_NOPROPERTIES()
    A_NOMETHODS()

public:
    ComputeShaderMt();

    void loadUserData(const VariantMap &data) override;

    MTL::ComputePipelineState *getProgramState();

    TextureList &textures() { return m_textures; }

    BufferList &buffers() { return m_buffers; }

    uint32_t uniformSize() const;

protected:
    ComputeInstance *createInstance() override;

private:
    friend class ComputeInstanceMt;

    TString m_shaderSource;

    MTL::ComputePipelineState *m_computePipeline;

};

#endif // COMPUTESHADERMT_H
