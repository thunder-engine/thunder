#ifndef MEDIASYSTEM_H
#define MEDIASYSTEM_H

#include <system.h>

#include <AL/alc.h>

class Camera;

class MediaSystem : public ISystem {
public:
    MediaSystem                 (Engine *engine);
    ~MediaSystem                ();

    bool                        init                        ();

    const char                 *name                        () const;

    void                        update                      (Scene &scene, uint32_t resource = 0);

    void                        overrideController          (IController *controller);

    void                        resize                      (uint32_t, uint32_t);

protected:
    Camera                     *activeCamera                ();

    ALCdevice                  *m_pDevice;
    ALCcontext                 *m_pContext;

    IController                *m_pController;
};

#endif // MEDIASYSTEM_H
