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
#ifndef RENDERVKSYSTEM_H
#define RENDERVKSYSTEM_H

#include <cstdint>

#include <systems/rendersystem.h>

#include "surfacevk.h"

class Engine;
class Viewport;

class RenderVkSystem : public RenderSystem {
public:
    RenderVkSystem(Engine *engine);
    ~RenderVkSystem();

    bool init() override;

    void update(World *world) override;

    void setCurrentSurface(SurfaceVk &surface);

    static int32_t swapChainImageCount();

#ifdef SHARED_DEFINE
    QWindow *createRhiWindow(Viewport *viewport) override;

#endif

private:
    Engine *m_engine;

    SurfaceVk *m_currentSurface;

};

#endif // RENDERVKSYSTEM_H
