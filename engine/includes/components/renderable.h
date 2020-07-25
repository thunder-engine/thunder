#ifndef RENDERABLE_H
#define RENDERABLE_H

#include "nativebehaviour.h"

#include <amath.h>

class RenderablePrivate;

class NEXT_LIBRARY_EXPORT Renderable : public NativeBehaviour {
    A_REGISTER(Renderable, NativeBehaviour, General)

    A_NOPROPERTIES()
    A_NOMETHODS()

public:
    Renderable ();

    virtual void draw (ICommandBuffer &buffer, uint32_t layer);

    virtual void update ();

    virtual AABBox bound () const;

};

#endif // RENDERABLE_H
