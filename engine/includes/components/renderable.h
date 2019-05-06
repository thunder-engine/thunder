#ifndef RENDERABLE_H
#define RENDERABLE_H

#include "nativebehaviour.h"

class NEXT_LIBRARY_EXPORT Renderable : public NativeBehaviour {
    A_REGISTER(Renderable, NativeBehaviour, General)

    A_NOPROPERTIES()
    A_NOMETHODS()

public:
    virtual void draw(ICommandBuffer &buffer, int8_t layer) {
        A_UNUSED(buffer);
        A_UNUSED(layer);
    }

};

#endif // RENDERABLE_H
