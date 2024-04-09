#include "mediasystem.h"

#include <AL/al.h>

#include <log.h>

#include <engine.h>
#include <systems/resourcesystem.h>
#include <components/camera.h>
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

    AudioClip::registerClassFactory(Engine::resourceSystem());

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
        const char *name = alcGetString(nullptr, ALC_DEFAULT_DEVICE_SPECIFIER);

        m_device = alcOpenDevice(name);
        if(m_device) {
            m_context = alcCreateContext(m_device, nullptr);
            if(alcGetError(m_device) == AL_NO_ERROR) {
                alcMakeContextCurrent(m_context);
                m_inited = true;
            }
        }
    }
    return m_inited;
}

void MediaSystem::update(World *world) {
    PROFILE_FUNCTION();

    Camera *camera = Camera::current();
    if(camera) {
        Transform *t = camera->transform();

        alListenerfv(AL_POSITION, t->worldPosition().v);

        Quaternion rot = t->worldQuaternion();

        Vector3 dir = rot * Vector3(0.0f, 0.0f,-1.0f);
        Vector3 up  = rot * Vector3(0.0f, 1.0f, 0.0f);
        float orientation[] = { dir.x, dir.y, dir.z, up.x, up.y, up.z };

        alListenerfv(AL_ORIENTATION, orientation);

        if(Engine::isGameMode()) {
            for(auto it : m_objectList) {
                NativeBehaviour *comp = dynamic_cast<NativeBehaviour *>(it);
                if(comp && comp->isEnabled() && comp->world() == world) {
                    if(!comp->isStarted()) {
                        comp->start();
                        comp->setStarted(true);
                    }
                    comp->update();
                }
            }
        }
    }
}

int MediaSystem::threadPolicy() const {
    return Pool;
}
