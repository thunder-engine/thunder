#ifndef UISYSTEM_H
#define UISYSTEM_H

#include <system.h>

class Widget;

class UiSystem : public System {
public:
    UiSystem();
    ~UiSystem();

    void update(World *) override;

    int threadPolicy() const override;

};

#endif // UISYSTEM_H
