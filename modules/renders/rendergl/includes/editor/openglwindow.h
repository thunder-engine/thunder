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
#ifndef OPENGLWINDOW_H
#define OPENGLWINDOW_H

#include <QOpenGLWindow>

#include <viewport/legacyviewport.h>

class OpenGLWindow : public QOpenGLWindow {
    Q_OBJECT

public:
    OpenGLWindow(LegacyViewport *viewport) :
            m_viewport(viewport) {
        connect(this, SIGNAL(frameSwapped()), this, SLOT(update()));
    }

protected:
    void paintGL() override {
        QOpenGLWindow::paintGL();

        m_viewport->onDraw();
    }

private:
    LegacyViewport *m_viewport;

};

#endif // OPENGLWINDOW_H
