#include "editor/editortool.h"

#include "components/actor.h"
#include "components/transform.h"
#include "components/renderable.h"

EditorTool::EditorTool(EditorTool::SelectMap &selection) :
    m_Selected(selection) {

}

void EditorTool::update() {

}

void EditorTool::beginControl() {

}

void EditorTool::endControl() {

}

QCursor EditorTool::cursor() const {
    return m_Cursor;
}

Vector3 EditorTool::objectPosition() {
    Vector3 result;
    if(!m_Selected.empty()) {
        for(auto &it : m_Selected) {
            result += it.object->transform()->worldPosition();
        }
        result = result / m_Selected.size();
    }
    return result;
}

AABBox EditorTool::objectBound() {
    AABBox result;
    bool first = true;
    if(!m_Selected.empty()) {
        for(auto &it : m_Selected) {
            Renderable *renderable = dynamic_cast<Renderable *>(it.object->component("Renderable"));
            if(renderable) {
                if(first) {
                    result = renderable->bound();
                    first = false;
                } else {
                    result.encapsulate(renderable->bound());
                }
            }
        }
    }
    return result;
}
