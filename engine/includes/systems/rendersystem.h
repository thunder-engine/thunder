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

#ifndef RENDERSYSTEM_H
#define RENDERSYSTEM_H

#include "engine.h"
#include "system.h"

class PipelineContext;
class Renderable;
class PostProcessVolume;

#if defined(SHARED_DEFINE)
class QWindow;
class LegacyViewport;
#endif

class ENGINE_EXPORT RenderSystem : public System {
public:
    RenderSystem();
    ~RenderSystem();

    bool init() override;

    void update(World *world) override;

    int threadPolicy() const override;

    PipelineContext *pipelineContext() const;
    void setPipelineContext(PipelineContext *context);

#if defined(SHARED_DEFINE)
    virtual QWindow *createRhiWindow(LegacyViewport *viewport);
#endif

    static void *windowHandle();
    static void setWindowHandle(void *handle);

protected:
    static void *m_windowHandle;

private:
    PipelineContext *m_pipelineContext;

    bool m_frameDirty;
};

#endif // RENDERSYSTEM_H
