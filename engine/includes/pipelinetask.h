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

#ifndef PIPELINETASK_H
#define PIPELINETASK_H

#include <renderable.h>
#include <pipelinecontext.h>
#include <engine.h>

class Texture;

class ENGINE_EXPORT PipelineTask : public Object {
    A_OBJECT(PipelineTask, Object, System)

public:
    PipelineTask();
    ~PipelineTask();

    virtual void setContext(PipelineContext *context);

    virtual void analyze(World *world);
    virtual void exec();

    virtual void resize(int width, int height);

    int inputCount() const;
    TString inputName(int index) const;
    virtual void setInput(int index, Texture *source);

    int outputCount() const;
    TString outputName(int index) const;
    virtual Texture *output(int index);

    virtual void setEnabled(bool enable);
    bool isEnabled() const;

protected:
    std::vector<TString> m_inputs;
    std::vector<std::pair<TString, Texture *>> m_outputs;

    PipelineContext *m_context;

    int m_width;
    int m_height;

    bool m_enabled;

};

#endif // PIPELINETASK_H
