#include "selecttool.h"

#include <components/actor.h>
#include <components/transform.h>

#include <editor/viewport/handles.h>

#include "../objectcontroller.h"

SelectTool::SelectTool(ObjectController *controller, SelectList &selection) :
        EditorTool(selection),
        m_controller(controller) {

}

void SelectTool::beginControl() {
    EditorTool::beginControl();

    m_position = objectPosition();
    m_savedWorld = m_world;
}

QString SelectTool::icon() const {
    return ":/Images/editor/Select.png";
}

QString SelectTool::name() const {
    return "Select";
}
