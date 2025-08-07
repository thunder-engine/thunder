#ifndef KEYFRAMEEDITOR_H
#define KEYFRAMEEDITOR_H

#include <QWidget>

#include <editor/undostack.h>

#include "timelinescene.h"
#include "animationclipmodel.h"

#include <animationcurve.h>

class TreeRow;
class QGraphicsView;
class QSplitter;

class KeyFrameEditor : public QWidget {
    Q_OBJECT

public:
    KeyFrameEditor(QWidget *parent = nullptr);
    ~KeyFrameEditor();

    void setModel(AnimationClipModel *model);

    void setReadOnly(bool flag);
    void setPosition(uint32_t position);

signals:
    void keySelectionChanged(int row, int col, int index);

    void rowsSelected(QStringList list);

    void headPositionChanged(uint32_t position);

public slots:
    void onDeleteSelectedKey();

private slots:
    void onClipUpdated();

    void onRemoveProperties();

    void onKeyPositionChanged(float delta);

    void onInsertKeyframe(int row, float position);

private:
    void createTree(const QModelIndex &parentIndex, TreeRow *parent, QList<TreeRow *> &items);

    void resizeEvent(QResizeEvent *event);

    void readSettings();
    void writeSettings();

private:
    QSplitter *m_splitter;

    AnimationClipModel *m_model;

    TimelineScene *m_scene;

    QGraphicsView *m_treeHeader;
    QGraphicsView *m_treeView;
    QGraphicsView *m_timelineHeader;
    QGraphicsView *m_timelineView;
};

class UndoKeyPositionChanged : public UndoCommand {
public:
    explicit UndoKeyPositionChanged(float delta, TimelineScene *scene, const TString &name, UndoCommand *parent = nullptr) :
            UndoCommand(name, parent),
            m_scene(scene),
            m_delta(delta) {

    }
    void undo() override;
    void redo() override;

protected:
    TimelineScene *m_scene;

    float m_delta;

};

class UndoDeleteSelectedKey : public UndoCommand {
public:
    UndoDeleteSelectedKey(TimelineScene *scene, const TString &name, UndoCommand *parent = nullptr) :
            UndoCommand(name, parent),
            m_scene(scene) {

    }
    void undo() override;
    void redo() override;

protected:
    struct FrameData {
        AnimationCurve::KeyFrame key;
        int row;
        int index;
    };

    std::list<FrameData> m_keys;

    TimelineScene *m_scene;

};

#endif // KEYFRAMEEDITOR_H
