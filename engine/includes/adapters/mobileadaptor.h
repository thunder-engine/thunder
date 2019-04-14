#ifndef DESKTOPAADAPTOR_H
#define DESKTOPAADAPTOR_H

#include "iplatformadaptor.h"

class Log;

class MobileAdaptor : public IPlatformAdaptor {
public:
    MobileAdaptor               (Engine *engine);

    virtual ~MobileAdaptor      () {}

    bool                        init                        ();

    void                        update                      ();

    bool                        start                       ();

    void                        stop                        ();

    void                        destroy                     ();

    bool                        isValid                     ();

    uint32_t                    screenWidth                 ();

    uint32_t                    screenHeight                ();

    uint32_t                    touchCount                  ();

    uint32_t                    touchState                  (uint32_t index);

    Vector4                     touchPosition               (uint32_t index);

public:
    static Vector2              s_Screen;

};

#endif // DESKTOPAADAPTOR_H
