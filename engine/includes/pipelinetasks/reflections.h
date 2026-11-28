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

#ifndef REFLECTIONS_H
#define REFLECTIONS_H

#include "pipelinetask.h"

class RenderTarget;
class MaterialInstance;

class Reflections : public PipelineTask {
    A_OBJECT(Reflections, PipelineTask, Pipeline)

public:
    Reflections();

private:
    void exec() override;

    void setInput(int index, Texture *texture) override;

private:
    MaterialInstance *m_sslrMaterial;

    Texture *m_sslrTexture;

    RenderTarget *m_sslrTarget;

};

#endif // REFLECTIONS_H
