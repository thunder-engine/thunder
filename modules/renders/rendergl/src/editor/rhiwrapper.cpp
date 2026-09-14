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
#include "editor/rhiwrapper.h"

#include "editor/openglwindow.h"

#include <QOpenGLContext>
#include <QOffscreenSurface>

QWindow *createWindow(Viewport *viewport) {
    return new OpenGLWindow(viewport);
}

void makeCurrent() {
    static QOffscreenSurface *surface = nullptr;
    static QOpenGLContext *context = nullptr;

    if(surface == nullptr) {
        surface = new QOffscreenSurface();
        surface->create();

        context = new QOpenGLContext();
        context->setShareContext(QOpenGLContext::globalShareContext());
        context->setFormat(surface->requestedFormat());
        context->create();

    }
    context->makeCurrent(surface);
}
