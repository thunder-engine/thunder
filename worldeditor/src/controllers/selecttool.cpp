#include "selecttool.h"

#include <components/actor.h>
#include <components/transform.h>

#include <editor/handles.h>

#include "objectctrl.h"
#include "undomanager.h"

SelectTool::SelectTool(ObjectCtrl *controller, SelectList &selection) :
    EditorTool(selection),
    m_pController(controller) {

}

void SelectTool::beginControl() {
    EditorTool::beginControl();

    m_Position = objectPosition();
    m_SavedWorld = m_World;
}

QString SelectTool::icon() const {
    return ":/Images/editor/Select.png";
}

QString SelectTool::name() const {
    return "Select";
}
