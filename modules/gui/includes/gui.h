#ifndef GUI_H
#define GUI_H

#include <module.h>

class GuiSystem;

class Gui : public Module {
public:
    Gui(Engine *engine);
    ~Gui() override;

    const char *metaInfo() const override;

    System *system(const char *name) override;

private:
    GuiSystem *m_system;

};

extern "C" {
    MODULE_EXPORT Module *moduleCreate(Engine *engine);
}

#endif // GUI_H
