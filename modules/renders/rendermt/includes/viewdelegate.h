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
#ifndef VIEWDELEGATE_H
#define VIEWDELEGATE_H

#include "wrappermt.h"

class RenderMtSystem;
class Viewport;

class ViewDelegate : public MTK::ViewDelegate {
public:
    explicit ViewDelegate(RenderMtSystem *system, Viewport *viewport);

private:
    void drawInMTKView(MTK::View *view) override;

private:
    RenderMtSystem *m_render;

    Viewport *m_viewport;

    bool m_captureInprogress = false;

    int frame = 0;

};

#endif // VIEWDELEGATE_H
