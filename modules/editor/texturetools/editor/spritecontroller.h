#ifndef SPRITECONTROLLER_H
#define SPRITECONTROLLER_H

#include <editor/undomanager.h>
#include <editor/viewport/cameractrl.h>

#include "../converter/textureconverter.h"

#define SCALE 100.0f

class QInputEvent;
class EditorPipeline;
class TextureImportSettings;

class SpriteController : public CameraCtrl {
    Q_OBJECT
public:
    explicit SpriteController(QWidget *view);

    void setImportSettings(TextureImportSettings *settings);

    void setSize(uint32_t width, uint32_t height);

    void selectElements(const QStringList &list);
    QStringList &selectedElements();

    TextureImportSettings *settings() const { return m_settings; }

    void onInputEvent(QInputEvent *) override;

signals:
    void selectionChanged(const QString &key);

    void setCursor(const QCursor &cursor);
    void unsetCursor();

private:
    void drawHandles() override;

    QPoint mapToScene(const QPoint &screen);

    QRect makeRect(const QPoint &p1, const QPoint &p2);
    QRectF mapRect(const QRectF &rect);

private:
    QPoint m_startPoint;
    QPoint m_currentPoint;
    QPoint m_save;

    TextureImportSettings *m_settings;

    uint32_t m_width;
    uint32_t m_height;

    QStringList m_list;
    QList<TextureImportSettings::Element> m_elementList;

    bool m_drag;
};

class UndoSprite : public QUndoCommand {
public:
    UndoSprite(SpriteController *ctrl, const QString &name, QUndoCommand *group = nullptr) :
            QUndoCommand(name, group) {
        m_controller = ctrl;
    }
protected:
    SpriteController *m_controller;
};

class SelectSprites : public UndoSprite {
public:
    SelectSprites(const QStringList &elements, SpriteController *ctrl, const QString &name = QObject::tr("Select Sprite Elements"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;
protected:
    QStringList m_list;
};

class CreateSprite : public UndoSprite {
public:
    CreateSprite(const TextureImportSettings::Element &rect, SpriteController *ctrl, QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;
protected:
    TextureImportSettings::Element m_rect;
    QString m_Uuid;
    QStringList m_list;
};

class DestroySprites : public UndoSprite {
public:
    DestroySprites(const QStringList &elements, SpriteController *ctrl, const QString &name = QObject::tr("Destroy Sprite Elements"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;
protected:
    QStringList m_list;
    QStringList m_uuids;
    QList<TextureImportSettings::Element> m_rects;
};

class UpdateSprites : public UndoSprite {
public:
    UpdateSprites(const QStringList &elements, const QList<TextureImportSettings::Element> &list, SpriteController *ctrl, const QString &name = QObject::tr("Update Sprite Elements"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;
protected:
    QStringList m_list;
    QStringList m_uuids;
    QList<TextureImportSettings::Element> m_rects;
};

#endif // SPRITECONTROLLER_H
