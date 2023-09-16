#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include <module.h>

class TextEditor : public Module {
public:
    TextEditor(Engine *engine);

    const char *metaInfo() const override;

    void *getObject(const char *name) override;

};

extern "C" {
    MODULE_EXPORT Module *moduleCreate(Engine *engine);
}

#endif // TEXTEDITOR_H
