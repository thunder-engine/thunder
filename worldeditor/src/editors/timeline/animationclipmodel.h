#ifndef ANIMATIONCLIPMODEL_H
#define ANIMATIONCLIPMODEL_H

#include <QAbstractItemModel>

#include <animationcurve.h>
#include <animationclip.h>

#include "undomanager.h"

class AnimationController;
class AnimationStateMachine;

class AnimationClipModel : public QAbstractItemModel {
    Q_OBJECT

public:
    AnimationClipModel(QObject *parent);

    void setController(AnimationController *controller);

    QVariant data(const QModelIndex &index, int) const override;

    QVariant headerData(int, Qt::Orientation, int) const override;

    int columnCount(const QModelIndex &) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex parent(const QModelIndex &) const override;

    int rowCount(const QModelIndex &) const override;

    float position() const;
    void setPosition(float value);

    void removeItems(const QModelIndexList &list);

    AnimationCurve::KeyFrame *key(int32_t track, int32_t col, int32_t index);

    AnimationTrack &track(int32_t track);

    void updateController();

    AnimationClip *clip() const { return m_pClip; }

    QStringList clips() const { return m_Clips.keys(); }

    void commitKey(int row, int col, int index, float value, float left, float right, int position);

    bool isReadOnly() const;

public slots:
    void setClip(const QString &clip);

signals:
    void changed();

    void positionChanged(float value);

protected:
    AnimationController *m_pController;

    AnimationStateMachine *m_pStateMachine;

    AnimationClip *m_pClip;

    float m_Position;

    QMap<QString, AnimationClip *> m_Clips;
};

class UndoUpdateKey : public QUndoCommand {
public:
    UndoUpdateKey(int row, int col, int index, float value, float left, float right, float position, AnimationClipModel *model, const QString &name, QUndoCommand *parent = nullptr) :
        QUndoCommand(name, parent),
        m_pModel(model),
        m_Row(row),
        m_Column(col),
        m_Index(index),
        m_Value(value),
        m_Left(left),
        m_Right(right),
        m_Position(position) {

    }
    void undo() override;
    void redo() override;
protected:
    AnimationClipModel *m_pModel;
    int m_Row;
    int m_Column;
    int m_Index;
    float m_Value;
    float m_Left;
    float m_Right;
    float m_Position;
    AnimationCurve::KeyFrame m_Key;
};

class UndoRemoveItems : public QUndoCommand {
public:
    UndoRemoveItems(QList<int> rows, AnimationClipModel *model, const QString &name, QUndoCommand *parent = nullptr) :
        QUndoCommand(name, parent),
        m_pModel(model),
        m_Rows(rows) {

    }
    void undo() override;
    void redo() override;
protected:
    AnimationClipModel *m_pModel;
    QList<int> m_Rows;
    AnimationTrackList m_Tracks;
};

class UndoUpdateItems : public QUndoCommand {
public:
    UndoUpdateItems(AnimationTrackList &tracks, AnimationClipModel *model, const QString &name, QUndoCommand *parent = nullptr) :
        QUndoCommand(name, parent),
        m_pModel(model),
        m_Tracks(tracks) {

    }
    void undo() override;
    void redo() override;
protected:
    AnimationClipModel *m_pModel;
    AnimationTrackList m_Tracks;
};

#endif // ANIMATIONCLIPMODEL_H
