#ifndef PREFAB_H
#define PREFAB_H

#include "resource.h"

class PrefabPrivate;

class Actor;

class NEXT_LIBRARY_EXPORT Prefab : public Resource {
    A_REGISTER(Prefab, Resource, Resources)

    A_PROPERTIES(
        A_PROPERTY(Actor *, Actor, Prefab::actor, Prefab::setActor)
    )

public:
    Prefab();
    ~Prefab();

    Actor *actor() const;
    void setActor(Actor *actor);

private:
    friend class ActorTest;

    PrefabPrivate *p_ptr;

};

#endif // PREFAB_H
