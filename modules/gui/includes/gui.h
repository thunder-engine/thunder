#ifndef GUI_H
#define GUI_H

#include <module.h>

#if defined(SHARED_DEFINE) && defined(_WIN32)
    #ifdef GUI_LIBRARY
        #define GUI_EXPORT __declspec(dllexport)
    #else
        #define GUI_EXPORT __declspec(dllimport)
    #endif
#else
    #define GUI_EXPORT
#endif

class GuiSystem;

class Gui : public Module {
public:
    Gui(Engine *engine);
    ~Gui() override;

    const char *metaInfo() const override;

    void *getObject(const char *name) override;

private:
    GuiSystem *m_system;

};

extern "C" {
    MODULE_EXPORT Module *moduleCreate(Engine *engine);
}

#endif // GUI_H
