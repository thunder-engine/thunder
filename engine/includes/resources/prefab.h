#ifndef PREFAB_H
#define PREFAB_H

#include "resource.h"

class Actor;

class ENGINE_EXPORT Prefab : public Resource {
    A_REGISTER(Prefab, Resource, Resources)

    A_PROPERTIES(
        A_PROPERTY(Actor *, actor, Prefab::actor, Prefab::setActor)
    )

public:
    Prefab();
    ~Prefab();

    Actor *actor() const;
    void setActor(Actor *actor);

private:
    friend class ActorTest;

    mutable Actor *m_actor;

};

#endif // PREFAB_H
