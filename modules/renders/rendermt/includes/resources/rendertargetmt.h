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
#ifndef RENDERTARGETMT_H
#define RENDERTARGETMT_H

#include <resources/rendertarget.h>

#include "wrappermt.h"

struct Global;

class RenderTargetMt : public RenderTarget {
    A_OBJECT_OVERRIDE(RenderTargetMt, RenderTarget, Resources)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    RenderTargetMt();

    MTL::RenderPassDescriptor *nativeHandle();
    void setNativeHandle(MTL::RenderPassDescriptor *descriptor);

    void setLevel(uint32_t level);

    MTL::Buffer *globalBuffer(size_t currentFrame);
    void updateGlobalMemory(size_t currentFrame, const Global &global);

    bool isNative() const;

private:
    std::vector<MTL::Buffer *> m_global;

    MTL::RenderPassDescriptor *m_descriptor;

    uint32_t m_currentLevel;

    bool m_native;

};

#endif // RENDERTARGETMT_H
