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

#ifndef DEFERREDINDIRECT_H
#define DEFERREDINDIRECT_H

#include "pipelinetask.h"

class RenderTarget;
class MaterialInstance;

class DeferredIndirect : public PipelineTask {
    A_OBJECT(DeferredIndirect, PipelineTask, Pipeline)

public:
    DeferredIndirect();

private:
    void analyze(World *world) override;
    void exec() override;

    void setInput(int index, Texture *texture) override;

private:
    Vector4 m_cameraColor;

    MaterialInstance *m_iblMaterial;

    Texture *m_cameraTexture;
    Texture *m_iblTexture;

    RenderTarget *m_iblTarget;

};

#endif // DEFERREDINDIRECT_H
