#ifndef MEDIASYSTEM_H
#define MEDIASYSTEM_H

#include <system.h>

#include <AL/alc.h>

class MediaSystem : public System {
public:
    MediaSystem();
    ~MediaSystem();

    bool init() override;

    void update(World *world) override;

    int threadPolicy() const override;

protected:
    ALCdevice  *m_device;
    ALCcontext *m_context;

    bool m_inited;
};

#endif // MEDIASYSTEM_H
