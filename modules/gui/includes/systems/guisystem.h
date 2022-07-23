#ifndef GUISYSTEM_H
#define GUISYSTEM_H

#include <system.h>

class GuiSystem : public System {
public:
    GuiSystem();
    ~GuiSystem();

    bool init() override;

private:
    const char *name() const override;

    void update(SceneGraph *) override;

    int threadPolicy() const override;

    Object *instantiateObject(const MetaObject *meta, const string &name, Object *parent) override;

    void composeComponent(Component *component) const override;

};

#endif // GUISYSTEM_H
