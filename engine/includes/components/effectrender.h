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

#ifndef EFFECTRENDER_H
#define EFFECTRENDER_H

#include "renderable.h"

#include "resources/visualeffect.h"

class ENGINE_EXPORT EffectRender : public Renderable {
    A_OBJECT(EffectRender, Renderable, Components/Effects)

    A_PROPERTIES(
        A_PROPERTYEX(VisualEffect *, effect, EffectRender::effect, EffectRender::setEffect, "editor=Asset")
    )
    A_NOMETHODS()

public:
    EffectRender();
    ~EffectRender() override;

    VisualEffect *effect() const;
    void setEffect(VisualEffect *effect);

    void deltaUpdate(float dt);

private:
    AABBox localBound() override;

    Mesh *meshToDraw() override;

    void update() override;

    static void effectUpdated(int state, void *ptr);

private:
    VisualEffect::Buffers m_data;

    VisualEffect *m_effect;

};

#endif // EFFECTRENDER_H
