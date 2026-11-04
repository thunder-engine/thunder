#include "destroysprite.h"

DestroySprite::DestroySprite(SpriteController *ctrl, UndoCommand *group) :
        UndoCommand(QObject::tr("Destroy Sprite Element").toStdString(), group),
        m_key(ctrl->selectedElement()),
        m_controller(ctrl) {
}
void DestroySprite::undo() {
    TextureImportSettings *settings = m_controller->settings();
    if(settings) {
        Sprite *sprite = new Sprite;
        ResourceSystem::loadResourceProperties(sprite, m_back);
        settings->setElement(sprite, m_key);

        m_controller->selectElement(m_key);
    }
}

void DestroySprite::redo() {
    TextureImportSettings *settings = m_controller->settings();
    if(settings) {
        Sprite *sprite = settings->element(m_key);
        if(sprite) {
            m_back = ResourceSystem::saveResourceProperties(sprite);
        }
        settings->removeElement(m_key, true);

        m_controller->selectElement({});
    }
}
