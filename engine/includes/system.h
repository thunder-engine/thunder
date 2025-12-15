#ifndef SYSTEM_H
#define SYSTEM_H

#include <engine.h>

class Component;

class ENGINE_EXPORT System : public ObjectSystem {
public:
    enum ThreadPolicy {
        Main = 0,
        Pool
    };

public:
    System();

    virtual bool init();

    virtual void reset();

    virtual void update(World *world) = 0;

    virtual int threadPolicy() const = 0;

    virtual void syncSettings() const;

    virtual void composeComponent(Component *component) const;

    void setActiveWorld(World *world);

    void processEvents() override;

protected:
    World *m_world;

};

#endif // SYSTEM_H
