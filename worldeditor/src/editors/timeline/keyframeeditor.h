#ifndef KEYFRAMEEDITOR_H
#define KEYFRAMEEDITOR_H

#include <QWidget>

#include <QUndoCommand>

#include <animationcurve.h>

class AnimationClipModel;
class TreeRow;
class TimelineScene;
class QGraphicsView;
class QSplitter;
class AnimationCurve;

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

    void onInsertKeyframe(int row, int col, float position);

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

class UndoInsertKey : public QUndoCommand {
public:
    explicit UndoInsertKey(int row, int col, float pos, AnimationClipModel *model, const QString &name, QUndoCommand *parent = nullptr) :
        QUndoCommand(name, parent),
        m_model(model),
        m_row(row),
        m_column(col),
        m_position(pos) {

    }
    void undo() override;
    void redo() override;

protected:
    void insertKey(AnimationCurve &curve);

protected:
    AnimationClipModel *m_model;
    int m_row;
    int m_column;
    float m_position;
    QList<int> m_indices;
};

class UndoKeyPositionChanged : public QUndoCommand {
public:
    explicit UndoKeyPositionChanged(float delta, TimelineScene *scene, const QString &name, QUndoCommand *parent = nullptr) :
        QUndoCommand(name, parent),
        m_delta(delta),
        m_scene(scene) {

    }
    void undo() override;
    void redo() override;

 protected:
    float m_delta;
    TimelineScene *m_scene;
};

class UndoDeleteSelectedKey : public QUndoCommand {
public:
    UndoDeleteSelectedKey(TimelineScene *scene, const QString &name, QUndoCommand *parent = nullptr) :
        QUndoCommand(name, parent),
        m_scene(scene) {

    }
    void undo() override;
    void redo() override;
protected:
    struct FrameData {
        AnimationCurve::KeyFrame key;
        int row;
        int column;
        int index;
    };

    TimelineScene *m_scene;
    QList<FrameData> m_keys;
};

#endif // KEYFRAMEEDITOR_H
