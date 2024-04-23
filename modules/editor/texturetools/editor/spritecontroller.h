#ifndef SPRITECONTROLLER_H
#define SPRITECONTROLLER_H

#include <editor/undomanager.h>
#include <editor/viewport/cameracontroller.h>

#include "../converter/textureconverter.h"

class SpriteController : public CameraController {
    Q_OBJECT

public:
    explicit SpriteController(QWidget *view);

    TextureImportSettings *settings() const { return m_settings; }
    void setSettings(TextureImportSettings *settings) { m_settings = settings; }

    void setSize(uint32_t width, uint32_t height);

    void selectElements(const list<string> &list);
    const list<string> &selectedElements();

signals:
    void selectionChanged(const QString &key);

private:
    void update() override;

    void drawHandles() override;

private:
    list<string> m_list;
    list<TextureImportSettings::Element> m_elementList;

    Vector3 m_startPoint;
    Vector3 m_currentPoint;

    TextureImportSettings *m_settings;

    Vector2 m_min;
    Vector2 m_max;

    Vector2 m_borderMin;
    Vector2 m_borderMax;

    uint32_t m_width;
    uint32_t m_height;

    uint8_t m_borderAxes;

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
    SelectSprites(const list<string> &elements, SpriteController *ctrl, const QString &name = QObject::tr("Select Sprite Elements"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    list<string> m_list;

};

class CreateSprite : public UndoSprite {
public:
    CreateSprite(const TextureImportSettings::Element &rect, SpriteController *ctrl, QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    TextureImportSettings::Element m_element;
    string m_uuid;
    list<string> m_list;

};

class DestroySprites : public UndoSprite {
public:
    DestroySprites(const list<string> &elements, SpriteController *ctrl, const QString &name = QObject::tr("Destroy Sprite Elements"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    list<string> m_list;
    list<string> m_uuids;
    list<TextureImportSettings::Element> m_elements;

};

class UpdateSprites : public UndoSprite {
public:
    UpdateSprites(const list<string> &elements, const list<TextureImportSettings::Element> &list, SpriteController *ctrl, const QString &name = QObject::tr("Update Sprite Elements"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    list<string> m_list;
    list<string> m_uuids;
    list<TextureImportSettings::Element> m_elements;

};

#endif // SPRITECONTROLLER_H
