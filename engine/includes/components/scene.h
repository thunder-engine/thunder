#ifndef SCENE_H
#define SCENE_H

#include "engine.h"

class NEXT_LIBRARY_EXPORT Scene : public Object {
    A_REGISTER(Scene, Object, General)

public:
    Scene               ();

    void                update              ();

    float               ambient             () const;
    void                setAmbient          (float ambient);

protected:
    void                updateComponents    (Object &object);

    /// Scene settings
    /// Power of ambient light.
    float               m_Ambient;

};

#endif // SCENE_H
