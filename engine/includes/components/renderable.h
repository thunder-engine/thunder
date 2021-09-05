#ifndef RENDERABLE_H
#define RENDERABLE_H

#include "nativebehaviour.h"

#include <amath.h>

class RenderablePrivate;
class CommandBuffer;

class NEXT_LIBRARY_EXPORT Renderable : public NativeBehaviour {
    A_REGISTER(Renderable, NativeBehaviour, General)

    A_NOPROPERTIES()
    A_METHODS(
        A_METHOD(AABBox, Renderable::bound)
    )

public:
    Renderable();

    virtual void draw(CommandBuffer &buffer, uint32_t layer);

    virtual AABBox bound() const;

    virtual bool isLight() const;

private:
    bool isRenderable() const override;

};

typedef list<Renderable *> RenderList;

#endif // RENDERABLE_H
