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

#ifndef DEPTHOFFIELD_H
#define DEPTHOFFIELD_H

#include "pipelinetask.h"

class RenderTarget;
class MaterialInstance;

class DepthOfField : public PipelineTask {
    A_OBJECT(DepthOfField, PipelineTask, Pipeline)

public:
    DepthOfField();
    ~DepthOfField();

private:
    void analyze(World *world) override;

    void exec() override;

    void resize(int32_t width, int32_t height) override;

    void setInput(int index, Texture *texture) override;

    void generateKernel(float radius, int32_t steps, float *points);

private:
    Texture *m_downTexture = nullptr;

    Texture *m_blurTexture = nullptr;

    Texture *m_resultTexture = nullptr;

    RenderTarget *m_downTarget = nullptr;

    RenderTarget *m_blurTarget = nullptr;

    RenderTarget *m_resultTarget = nullptr;

    MaterialInstance *m_downMaterial;

    MaterialInstance *m_blurMaterial;

    MaterialInstance *m_dofMaterial;

    float m_focusDistance;

    float m_focusScale;

    float m_blurSize;

    float m_skyDistance;

};

#endif // DEPTHOFFIELD_H
