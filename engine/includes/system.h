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

    virtual const char *name() const = 0;

    virtual void update(Scene *scene) = 0;

    virtual int threadPolicy() const = 0;

    virtual void syncSettings() const;

    virtual void composeComponent(Component *component) const;

    void setActiveScene(Scene *scene);

    void processEvents() override;

protected:
    Scene *m_pScene;

};

#endif // SYSTEM_H
