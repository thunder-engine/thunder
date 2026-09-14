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
#ifndef GUILAYER_H
#define GUILAYER_H

#include <amath.h>

#include "pipelinetask.h"

class Canvas;

class GuiLayer : public PipelineTask {
    A_OBJECT(GuiLayer, PipelineTask, Pipeline)

public:
    GuiLayer();

private:
    void analyze(World *world) override;

    void exec() override;

    void setInput(int index, Texture *source) override;

private:
    std::list<Canvas *> m_canvas;

};

#endif // GUILAYER_H

