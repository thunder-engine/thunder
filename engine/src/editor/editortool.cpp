#include "editor/editortool.h"

#include "components/actor.h"
#include "components/transform.h"
#include "components/renderable.h"
#include "components/camera.h"

#include "editor/viewport/handletools.h"

EditorTool::EditorTool() :
        m_cursor(Qt::ArrowCursor) {

}

EditorTool::~EditorTool() {

}

std::string EditorTool::toolTip() const {
    return std::string();
}

std::string EditorTool::shortcut() const {
    return std::string();
}

std::string EditorTool::component() const {
    return std::string();
}

bool EditorTool::blockSelection() const {
    return false;
}

QWidget *EditorTool::panel() {
    return nullptr;
}

void EditorTool::update(bool center, bool local, bool snap) {
    A_UNUSED(center);
    A_UNUSED(local);
    A_UNUSED(snap);
}

void EditorTool::beginControl() {

}

void EditorTool::endControl() {

}

void EditorTool::cancelControl() {

}

Qt::CursorShape EditorTool::cursor() const {
    return m_cursor;
}
