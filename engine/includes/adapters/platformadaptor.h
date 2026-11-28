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

#ifndef PLATFORMADAPTER_H
#define PLATFORMADAPTER_H

#include <cstdint>

#include <amath.h>

#include "input.h"

extern int thunderMain();

class Window;

class ENGINE_EXPORT PlatformAdaptor {
public:
    virtual ~PlatformAdaptor() {}

    virtual bool init() = 0;

    virtual void update() = 0;

    virtual bool start() = 0;

    virtual void loop();

    virtual void destroy() = 0;

    virtual bool isActive() const = 0;

    virtual uint32_t screenWidth() const = 0;
    virtual uint32_t screenHeight() const = 0;

    virtual Window *window();
    virtual void setWindow(Window *window);

    virtual bool key(Input::KeyCode code) const;
    virtual bool keyPressed(Input::KeyCode code) const;
    virtual bool keyReleased(Input::KeyCode code) const;

    virtual TString inputString() const = 0;
    virtual void setKeyboardVisible(bool visible);

    virtual Vector4 mousePosition() const;

    virtual Vector4 mouseDelta() const;
    virtual float mouseScrollDelta() const;

    virtual bool mouseButton(int code) const;
    virtual bool mousePressed(int code) const;
    virtual bool mouseReleased(int code) const;
    virtual bool mouseButtonDoubleClick(int code) const;
    virtual void mouseLockCursor(bool lock);
    virtual void mouseSetCursor(Input::CursorShape shape);

    virtual uint32_t joystickCount() const;
    virtual uint32_t joystickButtons(int index) const;
    virtual Vector4 joystickThumbs(int index) const;
    virtual Vector2 joystickTriggers(int index) const;

    virtual uint32_t touchCount() const;
    virtual uint32_t touchState(int index) const;
    virtual Vector4 touchPosition(int index) const;

    virtual TString locationLocalDir() const;

    virtual void syncConfiguration(VariantMap &map) const;

};

#endif // PLATFORMADAPTER_H
