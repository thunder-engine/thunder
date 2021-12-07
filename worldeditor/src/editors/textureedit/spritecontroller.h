#ifndef SPRITECONTROLLER_H
#define SPRITECONTROLLER_H

#include <editor/viewport/cameractrl.h>

#include "textureconverter.h"

#include <undomanager.h>

#define SCALE 100.0f

class QInputEvent;
class EditorPipeline;
class TextureImportSettings;

class SpriteController : public CameraCtrl {
    Q_OBJECT
public:
    explicit SpriteController(QWidget *view);
    ~SpriteController();

    void setImportSettings(TextureImportSettings *settings);

    void setSize(uint32_t width, uint32_t height);

    void selectElements(const QStringList &list);
    QStringList &selectedElements();

    TextureImportSettings *settings() const { return m_pSettings; }

    void init();

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
    QPoint m_StartPoint;
    QPoint m_CurrentPoint;
    QPoint m_Save;

    EditorPipeline *m_pPipeline;

    TextureImportSettings *m_pSettings;

    uint32_t m_Width;
    uint32_t m_Height;

    QStringList m_List;
    QList<TextureImportSettings::Element> m_ElementList;

    bool m_Drag;
};

class UndoSprite : public QUndoCommand {
public:
    UndoSprite(SpriteController *ctrl, const QString &name, QUndoCommand *group = nullptr) :
            QUndoCommand(name, group) {
        m_pController = ctrl;
    }
protected:
    SpriteController *m_pController;
};

class SelectSprites : public UndoSprite {
public:
    SelectSprites(const QStringList &elements, SpriteController *ctrl, const QString &name = QObject::tr("Select Sprite Elements"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;
protected:
    QStringList m_List;
};

class CreateSprite : public UndoSprite {
public:
    CreateSprite(const TextureImportSettings::Element &rect, SpriteController *ctrl, QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;
protected:
    TextureImportSettings::Element m_Rect;
    QString m_Uuid;
    QStringList m_List;
};

class DestroySprites : public UndoSprite {
public:
    DestroySprites(const QStringList &elements, SpriteController *ctrl, const QString &name = QObject::tr("Destroy Sprite Elements"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;
protected:
    QStringList m_List;
    QStringList m_Uuids;
    QList<TextureImportSettings::Element> m_Rects;
};

class UpdateSprites : public UndoSprite {
public:
    UpdateSprites(const QStringList &elements, const QList<TextureImportSettings::Element> &list, SpriteController *ctrl, const QString &name = QObject::tr("Update Sprite Elements"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;
protected:
    QStringList m_List;
    QStringList m_Uuids;
    QList<TextureImportSettings::Element> m_Rects;
};

#endif // SPRITECONTROLLER_H
