#include "mediasystem.h"

#include <AL/al.h>

#include <log.h>

#include <components/camera.h>
#include <components/actor.h>
#include <components/transform.h>

#include "components/audiosource.h"
#include "resources/audioclip.h"

MediaSystem::MediaSystem() :
        System(),
        m_device(nullptr),
        m_context(nullptr),
        m_inited(false) {
    PROFILE_FUNCTION();

    AudioSource::registerClassFactory(this);

    AudioClip::registerClassFactory(this);

    setName("Media");
}

MediaSystem::~MediaSystem() {
    PROFILE_FUNCTION();

    alcDestroyContext(m_context);
    alcCloseDevice(m_device);
}

bool MediaSystem::init() {
    PROFILE_FUNCTION();
    if(!m_inited) {
        m_device   = alcOpenDevice(nullptr);
        if(m_device) {
            m_context  = alcCreateContext(m_device, nullptr);
            if(alcGetError(m_device) == AL_NO_ERROR) {
                alcMakeContextCurrent(m_context);
                return m_inited;
            }
        }
    }
    return m_inited;
}

void MediaSystem::update(World *) {
    PROFILE_FUNCTION();

    Camera *camera  = Camera::current();
    if(camera) {
        Actor *a = camera->actor();
        Transform *t = a->transform();

        alListenerfv(AL_POSITION, t->worldPosition().v);

        Quaternion rot = t->worldQuaternion();

        Vector3 dir = rot * Vector3(0.0f, 0.0f,-1.0f);
        Vector3 up  = rot * Vector3(0.0f, 1.0f, 0.0f);
        float orientation[] = { dir.x, dir.y, dir.z, up.x, up.y, up.z };

        alListenerfv(AL_ORIENTATION, orientation);
    }
}

int MediaSystem::threadPolicy() const {
    return Pool;
}
