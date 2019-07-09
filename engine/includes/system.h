#ifndef SYSTEM_H
#define SYSTEM_H

#include <string>
#include <stdint.h>

#include <objectsystem.h>

class Scene;

class NEXT_LIBRARY_EXPORT ISystem : public ObjectSystem {
public:
    ISystem();

    virtual bool init () = 0;

    virtual const char *name () const = 0;

    virtual void update (Scene *scene) = 0;

    virtual bool isThreadFriendly() const = 0;

    void setActiveScene (Scene *scene);

private:
    void processEvents () override;

private:
    Scene *m_pScene;

};

#endif // SYSTEM_H
