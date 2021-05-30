#include <amath.h>
#include <engine.h>

#include "gui.h"

#include "systems/guisystem.h"

Module *moduleCreate(Engine *) {
    return new Gui();
}

Gui::Gui() :
        Module(),
        m_system(new GuiSystem) {
}

Gui::~Gui() {
    delete m_system;
}

System *Gui::system() {
    return m_system;
}

const char *Gui::description() const {
    return "GUI Plugin";
}

const char *Gui::version() const {
    return "1.0";
}

uint8_t Gui::types() const {
    return SYSTEM;
}
