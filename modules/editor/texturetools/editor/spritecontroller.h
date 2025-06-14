#ifndef SPRITECONTROLLER_H
#define SPRITECONTROLLER_H

#include <editor/undomanager.h>
#include <editor/viewport/cameracontroller.h>

#include "../converter/textureconverter.h"

#include "tools/spritetool.h"

class SpriteController : public CameraController {
    Q_OBJECT

public:
    SpriteController();

    TextureImportSettings *settings() const { return m_settings; }
    void setSettings(TextureImportSettings *settings);

    void setSize(uint32_t width, uint32_t height);
    uint32_t width() const { return m_width; }
    uint32_t height() const { return m_height; }

    bool isSelected(const std::string &key) const;
    void selectElement(const std::string &key);
    std::string selectedElement();

    bool isDrag() const { return m_drag; }
    void setDrag(bool drag);

    Vector3 world() const;

signals:
    void itemsSelected(std::list<QObject *>);

    void updated();

private:
    void update() override;

    void drawHandles() override;

private:
    std::string m_key;

    TextureImportSettings *m_settings;

    EditorTool *m_spriteTool;

    uint32_t m_width;
    uint32_t m_height;

    Vector3 m_world;

    bool m_drag;
};

class UndoSprite : public UndoCommand {
public:
    UndoSprite(SpriteController *ctrl, const QString &text, QUndoCommand *parent = nullptr) :
            UndoCommand(text, ctrl, parent),
            m_controller(ctrl) {

    }

protected:
    SpriteController *m_controller;

};

class SelectSprite : public UndoSprite {
public:
    SelectSprite(const std::string &key, SpriteController *ctrl, const QString &name = QObject::tr("Select Sprite Elements"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    std::string m_key;

};

class CreateSprite : public UndoSprite {
public:
    CreateSprite(const TextureImportSettings::Element &element, SpriteController *ctrl, QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    TextureImportSettings::Element m_element;
    std::string m_uuid;
    std::string m_key;

};

class DestroySprite : public UndoSprite {
public:
    DestroySprite(SpriteController *ctrl, const QString &name = QObject::tr("Destroy Sprite Element"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    std::string m_key;
    TextureImportSettings::Element m_element;

};

class UpdateSprite : public UndoSprite {
public:
    UpdateSprite(const TextureImportSettings::Element &element, SpriteController *ctrl, const QString &name = QObject::tr("Update Sprite Element"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    std::string m_key;
    TextureImportSettings::Element m_element;

};

class RenameSprite : public UndoSprite {
public:
    RenameSprite(const std::string &oldKey, const std::string &newKey, SpriteController *ctrl, const QString &name = QObject::tr("Rename Sprite Element"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    std::string m_newKey;
    std::string m_oldKey;
};

#endif // SPRITECONTROLLER_H
