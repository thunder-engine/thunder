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

#include <platformadaptor.h>

Window *PlatformAdaptor::window() {
    return nullptr;
}

void PlatformAdaptor::setWindow(Window *window) {
    A_UNUSED(window);
}

void PlatformAdaptor::update() {

}

void PlatformAdaptor::loop() {

}

bool PlatformAdaptor::key(Input::KeyCode code) const {
    A_UNUSED(code);
    return false;
}

bool PlatformAdaptor::keyPressed(Input::KeyCode code) const {
    A_UNUSED(code);
    return false;
}

bool PlatformAdaptor::keyReleased(Input::KeyCode code) const {
    A_UNUSED(code);
    return false;
}

void PlatformAdaptor::setKeyboardVisible(bool visible) {
    A_UNUSED(visible);
}

Vector4 PlatformAdaptor::mousePosition() const {
    return Vector4();
}

Vector4 PlatformAdaptor::mouseDelta() const {
    return Vector4();
}

float PlatformAdaptor::mouseScrollDelta() const {
    return 0;
}

bool PlatformAdaptor::mouseButton(int code) const {
    A_UNUSED(code);
    return false;
}

bool PlatformAdaptor::mousePressed(int code) const {
    A_UNUSED(code);
    return false;
}

bool PlatformAdaptor::mouseReleased(int code) const {
    A_UNUSED(code);
    return false;
}

bool PlatformAdaptor::mouseButtonDoubleClick(int code) const {
    A_UNUSED(code);
    return false;
}

void PlatformAdaptor::mouseLockCursor(bool lock) {
    A_UNUSED(lock);
}

void PlatformAdaptor::mouseSetCursor(Input::CursorShape shape) {
    A_UNUSED(shape);
}

uint32_t PlatformAdaptor::joystickCount() const {
    return 0;
}

uint32_t PlatformAdaptor::joystickButtons(int index) const {
    A_UNUSED(index);
    return 0;
}

Vector4 PlatformAdaptor::joystickThumbs(int index) const {
    A_UNUSED(index);
    return Vector4();
}

Vector2 PlatformAdaptor::joystickTriggers(int index) const {
    A_UNUSED(index);
    return Vector2();
}

uint32_t PlatformAdaptor::touchCount() const {
    return 0;
}

uint32_t PlatformAdaptor::touchState(int index) const {
    A_UNUSED(index);
    return 0;
}

Vector4 PlatformAdaptor::touchPosition(int index) const {
    A_UNUSED(index);
    return 0;
}

TString PlatformAdaptor::locationLocalDir() const {
    return TString();
}

void PlatformAdaptor::syncConfiguration(VariantMap &map) const {
    A_UNUSED(map);
}
