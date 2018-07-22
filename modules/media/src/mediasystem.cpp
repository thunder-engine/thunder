#include "mediasystem.h"

#include <AL/al.h>

#include <log.h>
#include <controller.h>

#include <analytics/profiler.h>
#include <components/camera.h>
#include <components/actor.h>

#include "components/audiosource.h"
#include "resources/audioclip.h"

MediaSystem::MediaSystem(Engine *engine) :
        m_pDevice(nullptr),
        m_pContext(nullptr),
        ISystem(engine) {
    PROFILER_MARKER;

    AudioSource::registerClassFactory();

    AudioClip::registerClassFactory();
}

MediaSystem::~MediaSystem() {
    PROFILER_MARKER;

    alcDestroyContext(m_pContext);
    alcCloseDevice(m_pDevice);
}

bool MediaSystem::init() {
    PROFILER_MARKER;

    m_pDevice   = alcOpenDevice(0);
    if(m_pDevice) {
        m_pContext  = alcCreateContext(m_pDevice, 0);
        if(alcGetError(m_pDevice) == AL_NO_ERROR) {
            alcMakeContextCurrent(m_pContext);

            return true;
        }
    }
    return false;
}

const char *MediaSystem::name() const {
    return "Media";
}

void MediaSystem::update(Scene &scene, uint32_t resource) {
    PROFILER_MARKER;

    Camera *camera  = activeCamera();
    Actor &a    = camera->actor();

    alListenerfv(AL_POSITION,    a.position().v);

    Vector3 dir = a.rotation() * Vector3(0.0f, 0.0f,-1.0f);
    Vector3 up  = a.rotation() * Vector3(0.0f, 1.0f, 0.0f);
    float orientation[] = { dir.x, dir.y, dir.z, up.x, up.y, up.z };

    alListenerfv(AL_ORIENTATION, orientation);
}

void MediaSystem::overrideController(IController *controller) {
    PROFILER_MARKER;

    m_pController   = controller;
}

void MediaSystem::resize(uint32_t, uint32_t) {
    PROFILER_MARKER;

}

Camera *MediaSystem::activeCamera() {
    if(m_pController) {
        return m_pController->activeCamera();
    }
    return m_pEngine->controller()->activeCamera();
}
