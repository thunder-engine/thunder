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
    A_OVERRIDE(ComputeShaderMt, ComputeShader, Resources)

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

    std::string m_shaderSource;

    MTL::ComputePipelineState *m_computePipeline;

};

#endif // COMPUTESHADERMT_H
