#include "texteditor.h"

#include <amath.h>
#include <engine.h>

#include <cstring>

#include "editor/textedit.h"

static const char *meta = \
"{"
"   \"module\": \"TextEditor\","
"   \"version\": \"1.0\","
"   \"description\": \"Code Editor plugin\","
"   \"author\": \"Evgeniy Prikazchikov\","
"   \"objects\": {"
"       \"TextEdit\": \"editor\""
"   }"
"}";

Module *moduleCreate(Engine *engine) {
    return new TextEditor(engine);
}

TextEditor::TextEditor(Engine *engine) :
        Module(engine) {
}

const char *TextEditor::metaInfo() const {
    return meta;
}

void *TextEditor::getObject(const char *name) {
    return new TextEdit;
}
