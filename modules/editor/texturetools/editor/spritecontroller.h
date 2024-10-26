#ifndef SPRITECONTROLLER_H
#define SPRITECONTROLLER_H

#include <editor/undomanager.h>
#include <editor/viewport/cameracontroller.h>

#include "../converter/textureconverter.h"

#include "tools/spritetool.h"

class SpriteController : public CameraController {
    Q_OBJECT

public:
    explicit SpriteController(QWidget *view);

    TextureImportSettings *settings() const { return m_settings; }
    void setSettings(TextureImportSettings *settings) { m_settings = settings; }

    void setSize(uint32_t width, uint32_t height);
    uint32_t width() const { return m_width; }
    uint32_t height() const { return m_height; }

    bool isSelected(const std::string &key) const;
    void selectElements(const std::list<std::string> &list);
    const std::list<std::string> &selectedElements();

    bool isDrag() const { return m_drag; }
    void setDrag(bool drag);

    Vector3 world() const;

signals:
    void selectionChanged(const QString &key);

private:
    void update() override;

    void drawHandles() override;

private:
    std::list<std::string> m_selected;

    TextureImportSettings *m_settings;

    SpriteTool *m_spriteTool;

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

class SelectSprites : public UndoSprite {
public:
    SelectSprites(const std::list<std::string> &list, SpriteController *ctrl, const QString &name = QObject::tr("Select Sprite Elements"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    std::list<std::string> m_list;

};

class CreateSprite : public UndoSprite {
public:
    CreateSprite(const TextureImportSettings::Element &rect, SpriteController *ctrl, QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    TextureImportSettings::Element m_element;
    std::string m_uuid;
    std::list<std::string> m_list;

};

class DestroySprites : public UndoSprite {
public:
    DestroySprites(SpriteController *ctrl, const QString &name = QObject::tr("Destroy Sprite Elements"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    std::list<std::string> m_list;
    std::list<std::string> m_uuids;
    std::list<TextureImportSettings::Element> m_elements;

};

class UpdateSprites : public UndoSprite {
public:
    UpdateSprites(const std::list<TextureImportSettings::Element> &list, SpriteController *ctrl, const QString &name = QObject::tr("Update Sprite Elements"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    std::list<std::string> m_list;
    std::list<std::string> m_uuids;
    std::list<TextureImportSettings::Element> m_elements;

};

#endif // SPRITECONTROLLER_H
