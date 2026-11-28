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

#ifndef BLOOM_H
#define BLOOM_H

#include <amath.h>

#include "pipelinetask.h"

#define MAX_SAMPLES 32

class RenderTarget;
class MaterialInstance;

class Bloom : public PipelineTask {
    A_OBJECT(Bloom, PipelineTask, Pipeline)

    struct BloomPass {
        RenderTarget *blurTempTarget = nullptr;

        RenderTarget *downTarget = nullptr;

        Texture *downTexture = nullptr;

        Texture *blurTempTexture = nullptr;

        MaterialInstance *blurMaterialV = nullptr;

        MaterialInstance *blurMaterialH = nullptr;

        float blurPoints[MAX_SAMPLES];

        int32_t steps;
    };

public:
    Bloom();
    ~Bloom();

private:
    void analyze(World *world) override;

    void exec() override;

    void resize(int32_t width, int32_t height) override;

    void setInput(int index, Texture *source) override;

    void generateKernel(float radius, int32_t steps, float *points);

private:
    std::vector<BloomPass> m_bloomPasses;

    RenderTarget *m_resultTarget = nullptr;

    MaterialInstance *m_thresholdMaterial = nullptr;

    MaterialInstance *m_downMaterial = nullptr;

    Texture *m_dirtTexture = nullptr;

    uint32_t m_mipLevels;

    float m_threshold;

    float m_intensity;

    float m_dirtIntensity;

};

#endif // BLOOM_H

