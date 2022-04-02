#ifndef CODERDITOR_H
#define CODERDITOR_H

#include <module.h>

class CodeEditor : public Module {
public:
    CodeEditor(Engine *engine);

    const char *metaInfo() const override;

    void *getObject(const char *name) override;

};

extern "C" {
    MODULE_EXPORT Module *moduleCreate(Engine *engine);
}

#endif // CODERDITOR_H
