#ifndef GUI_H
#define GUI_H

#include <module.h>

class GuiSystem;

class Gui : public Module {
public:
    Gui();
    ~Gui() override;

    const char *description() const override;

    const char *version() const override;

    uint32_t types() const override;

    System *system() override;

private:
    GuiSystem *m_system;

};

extern "C" {
    MODULE_EXPORT Module *moduleCreate(Engine *);
}

#endif // GUI_H
