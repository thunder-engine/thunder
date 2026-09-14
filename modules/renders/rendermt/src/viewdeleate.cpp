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
#include "viewdelegate.h"

#include "rendermtsystem.h"

#include <timer.h>

#if defined(SHARED_DEFINE)
#include <viewport/viewport.h>
#endif

ViewDelegate::ViewDelegate(RenderMtSystem *system, Viewport *viewport) :
        m_render(system),
        m_viewport(viewport) {

}

void ViewDelegate::drawInMTKView(MTK::View *view) {
    m_render->setCurrentView(view);

    // Render cycle here
#if defined(SHARED_DEFINE)
    m_viewport->onDraw();
#else
    Engine::update(Engine::world());
#endif
}
