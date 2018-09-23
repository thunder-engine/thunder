#include "adapters/mobileadaptor.h"

#include <glfm.h>

#include <log.h>

Vector4 MobileAdaptor::m_MouseDelta     = Vector4();
Vector4 MobileAdaptor::m_MousePosition  = Vector4();

void glfmMain(GLFMDisplay *display) {
    glfmSetDisplayConfig(display,
                         GLFMRenderingAPIOpenGLES2,
                         GLFMColorFormatRGBA8888,
                         GLFMDepthFormatNone,
                         GLFMStencilFormatNone,
                         GLFMMultisampleNone);
    //glfmSetSurfaceCreatedFunc(display, onSurfaceCreated);
    //glfmSetSurfaceResizedFunc(display, onSurfaceCreated);
    //glfmSetSurfaceDestroyedFunc(display, onSurfaceDestroyed);
    //glfmSetMainLoopFunc(display, onFrame);
}

MobileAdaptor::MobileAdaptor(Engine *engine) {
    A_UNUSED(engine)
}

bool MobileAdaptor::init() {
    return true;
}

void MobileAdaptor::update() {

}

bool MobileAdaptor::start() {
    return true;
}

void MobileAdaptor::stop() {

}

void MobileAdaptor::destroy() {

}

bool MobileAdaptor::isValid() {
    return true;
}

bool MobileAdaptor::key(Input::KeyCode) {
    return false;
}

Vector4 MobileAdaptor::mousePosition() {
    return m_MousePosition;
}

Vector4 MobileAdaptor::mouseDelta() {
    return m_MouseDelta;
}

uint8_t MobileAdaptor::mouseButtons() {
    return m_MouseButtons;
}

uint32_t MobileAdaptor::screenWidth() {
    return 0;
}

uint32_t MobileAdaptor::screenHeight() {
    return 0;
}

void MobileAdaptor::setMousePosition(const Vector3 &) {
}

uint16_t MobileAdaptor::joystickCount() {
    return 0;
}

uint16_t MobileAdaptor::joystickButtons(uint8_t) {
    return 0;
}

Vector4 MobileAdaptor::joystickThumbs(uint8_t) {
    return Vector4();
}

Vector2 MobileAdaptor::joystickTriggers(uint8_t) {
    return Vector2();
}

void *MobileAdaptor::pluginLoad(const char *) {
    return nullptr;
}

bool MobileAdaptor::pluginUnload(void *) {
    return false;
}

void *MobileAdaptor::pluginAddress(void *, const string &) {
    return nullptr;
}

string MobileAdaptor::locationLocalDir() {
    string result;

    return result;
}
