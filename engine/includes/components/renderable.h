#ifndef RENDERABLE_H
#define RENDERABLE_H

#include "nativebehaviour.h"

#include <amath.h>

class CommandBuffer;

class ENGINE_EXPORT Renderable : public NativeBehaviour {
    A_REGISTER(Renderable, NativeBehaviour, General)

    A_NOPROPERTIES()
    A_METHODS(
        A_METHOD(AABBox, Renderable::bound)
    )

public:
    Renderable();
    ~Renderable();

    virtual void draw(CommandBuffer &buffer, uint32_t layer);

    virtual AABBox bound() const;

    virtual int priority() const;

private:
    void setSystem(ObjectSystem *system) override;

};

typedef list<Renderable *> RenderList;

#endif // RENDERABLE_H
