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

    uint16_t                    touchCount                  ();

    uint16_t                    touchState                  (uint8_t index);

    Vector2                     touchPosition               (uint8_t index);

public:
    static Vector2              s_Screen;

};

#endif // DESKTOPAADAPTOR_H
