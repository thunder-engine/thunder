#include "editor/editortool.h"

#include "components/actor.h"
#include "components/transform.h"
#include "components/renderable.h"
#include "components/camera.h"

#include "editor/viewport/handletools.h"

#include <QString>

EditorTool::EditorTool() :
    m_cursor(Qt::ArrowCursor),
    m_snap(0.0f) {

}

QString EditorTool::toolTip() const {
    return QString();
}

QString EditorTool::shortcut() const {
    return QString();
}

float EditorTool::snap() const {
    return m_snap;
}

void EditorTool::setSnap(float snap) {
    m_snap = snap;
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
