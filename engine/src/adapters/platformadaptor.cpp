#include <platformadaptor.h>

void PlatformAdaptor::update() {

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

void PlatformAdaptor::mouseLockCursor(bool lock) {
    A_UNUSED(lock);
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

void *PlatformAdaptor::pluginLoad(const char *name) {
    A_UNUSED(name);
    return nullptr;
}

bool PlatformAdaptor::pluginUnload(void *plugin) {
    A_UNUSED(plugin);
    return false;
}

void *PlatformAdaptor::pluginAddress(void *plugin, const TString &name) {
    A_UNUSED(plugin);
    A_UNUSED(name);
    return nullptr;
}

TString PlatformAdaptor::locationLocalDir() const {
    return TString();
}

void PlatformAdaptor::syncConfiguration(VariantMap &map) const {
    A_UNUSED(map);
}
