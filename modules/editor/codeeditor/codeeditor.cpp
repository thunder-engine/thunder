#include "codeeditor.h"

#include <amath.h>
#include <engine.h>

#include <cstring>

#include "editor/textedit.h"

static const char *meta = \
"{"
"   \"module\": \"CodeEditor\","
"   \"version\": \"1.0\","
"   \"description\": \"Code Editor plugin\","
"   \"author\": \"Evgeniy Prikazchikov\","
"   \"objects\": {"
"       \"CodeEdit\": \"editor\""
"   }"
"}";

Module *moduleCreate(Engine *engine) {
    return new CodeEditor(engine);
}

CodeEditor::CodeEditor(Engine *engine) :
        Module(engine) {
}

const char *CodeEditor::metaInfo() const {
    return meta;
}

void *CodeEditor::getObject(const char *name) {
    return new TextEdit;
}
