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
#ifndef COMPUTESHADERGL_H
#define COMPUTESHADERGL_H

#include <unordered_map>
#include <list>

#include <resources/computeshader.h>

#include <engine.h>

class CommandBufferGL;

class ComputeInstanceGL : public ComputeInstance {
public:
    ComputeInstanceGL(ComputeShader *compute);

    ~ComputeInstanceGL();

    bool bind(CommandBufferGL *buffer);

private:
    uint32_t m_instanceUbo;

};

class ComputeShaderGL : public ComputeShader {
    A_OBJECT_OVERRIDE(ComputeShaderGL, ComputeShader, Resources)

    A_NOPROPERTIES()
    A_NOMETHODS()

public:
    void loadUserData(const VariantMap &data) override;

    uint32_t getProgram();

    TextureList &textures() { return m_textures; }

    BufferList &buffers() { return m_buffers; }

    uint32_t uniformSize() const;

protected:
    uint32_t buildShader(const TString &src = TString());

    uint32_t buildProgram(uint32_t shader);

    bool checkShader(uint32_t shader, const TString &path, bool link = false);

    ComputeInstance *createInstance() override;

private:
    friend class ComputeInstanceGL;

    TString m_shaderSource;

    uint32_t m_program;

};

#endif // COMPUTESHADERGL_H
