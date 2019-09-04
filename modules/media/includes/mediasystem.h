#ifndef MEDIASYSTEM_H
#define MEDIASYSTEM_H

#include <system.h>

#include <AL/alc.h>

class MediaSystem : public ISystem {
public:
    MediaSystem                 ();
    ~MediaSystem                ();

    bool                        init                        ();

    const char                 *name                        () const;

    void                        update                      (Scene *);

    bool                        isThreadSafe                () const;

protected:
    ALCdevice                  *m_pDevice;
    ALCcontext                 *m_pContext;

    bool                        m_Inited;
};

#endif // MEDIASYSTEM_H
