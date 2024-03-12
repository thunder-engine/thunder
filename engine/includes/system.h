#ifndef SYSTEM_H
#define SYSTEM_H

#include <string>
#include <stdint.h>

#include <engine.h>

class Scene;
class Component;

class ENGINE_EXPORT System : public ObjectSystem {
public:
    enum ThreadPolicy {
        Main = 0,
        Pool
    };

public:
    System();

    virtual bool init() = 0;

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
