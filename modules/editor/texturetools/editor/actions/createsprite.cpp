#include "createsprite.h"

#include <object.h>

CreateSprite::CreateSprite(const Vector2 &min, const Vector2 &max, SpriteController *ctrl, UndoCommand *group) :
        UndoCommand(QObject::tr("Create Sprite Element").toStdString(), group),
        m_bounds(min, max),
        m_controller(ctrl) {
}

void CreateSprite::undo() {
    TextureImportSettings *settings = m_controller->settings();
    if(settings) {
        settings->removeElement(m_key, true);
        m_controller->selectElement(m_previous);
    }
}

void CreateSprite::redo() {
    TextureImportSettings *settings = m_controller->settings();
    if(settings) {
        Object::ObjectList list = m_controller->selected();
        if(!list.empty()) {
            m_previous = list.front()->name();
        }
        Sprite *element = new Sprite;
        element->setBounds(m_bounds);

        m_key = settings->setElement(element, m_key);
        m_controller->selectElement(m_key);
    }
}
