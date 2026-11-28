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

#ifndef AMBIENTOCCLUSION_H
#define AMBIENTOCCLUSION_H

#include "pipelinetask.h"

#include <amath.h>

class RenderTarget;
class MaterialInstance;

class AmbientOcclusion : public PipelineTask {
    A_OBJECT(AmbientOcclusion, PipelineTask, Pipeline)

public:
    AmbientOcclusion();
    ~AmbientOcclusion();

private:
    void analyze(World *world) override;
    void exec() override;

    void resize(int32_t width, int32_t height) override;

    void setEnabled(bool enable) override;

protected:
    Texture *m_noiseTexture;
    Texture *m_aoTexture;
    Texture *m_blurTexture;

    RenderTarget *m_aoTarget;
    RenderTarget *m_blurTarget;

    MaterialInstance *m_occlusion;
    MaterialInstance *m_blur;

    float m_radius;
    float m_bias;
    float m_power;

    bool m_toDisable;
};

#endif // AMBIENTOCCLUSION_H
