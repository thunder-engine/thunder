#include "updatesprite.h"

UpdateSprite::UpdateSprite(const VariantMap &data, SpriteController *ctrl, UndoCommand *group) :
        UndoCommand(QObject::tr("Update Sprite Element").toStdString(), group),
        m_key(ctrl->selectedElement()),
        m_data(data),
        m_controller(ctrl) {
}

void UpdateSprite::undo() {
    redo();
}

void UpdateSprite::redo() {
    TextureImportSettings *settings = m_controller->settings();
    if(settings) {
        Sprite *sprite = settings->element(m_key);
        if(sprite) {
            VariantMap temp = ResourceSystem::saveResourceProperties(sprite);
            ResourceSystem::loadResourceProperties(sprite, m_data);
            m_data = temp;

            m_controller->selectElement(m_key);
            m_controller->updated();
        }
    }
}
