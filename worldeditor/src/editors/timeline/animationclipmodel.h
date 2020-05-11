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

    Q_PROPERTY(float position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(int row READ row WRITE setRow NOTIFY rowChanged)
    Q_PROPERTY(int col READ col WRITE setCol NOTIFY rowChanged)

public:
    AnimationClipModel          (QObject *parent);

    void                        setController               (AnimationController *controller);

    QVariant                    data                        (const QModelIndex &index, int) const;

    QVariant                    headerData                  (int, Qt::Orientation, int) const;

    int                         columnCount                 (const QModelIndex &) const;

    QModelIndex                 index                       (int row, int column, const QModelIndex &parent = QModelIndex()) const;

    QModelIndex                 parent                      (const QModelIndex &) const;

    int                         rowCount                    (const QModelIndex &) const;

    Q_INVOKABLE bool            isExpanded                  (int track) const;

    Q_INVOKABLE int             expandHeight                (int track) const;

    Q_INVOKABLE QVariant        trackData                   (int track) const;

    Q_INVOKABLE void            setTrackData                (int track, const QVariant &data);

    Q_INVOKABLE int             maxPosition                 (int track);


    float                       position                    () const;
    void                        setPosition                 (float value);

    int                         row                         () const;
    void                        setRow                      (int value);

    int                         col                         () const;
    void                        setCol                      (int value);

    void                        selectItem                  (const QModelIndex &index);

    void                        removeItems                 (const QModelIndexList &list);

    AnimationCurve::KeyFrame   *key                         (int32_t track, int32_t col, int32_t index);

    void                        updateController            ();

    AnimationClip              *clip                        () const { return m_pClip; }

    QStringList                 clips                       () const { return m_Clips.keys(); }

    static bool compare(const AnimationCurve::KeyFrame &first, const AnimationCurve::KeyFrame &second) {
        return ( first.m_Position < second.m_Position );
    }

public slots:
    void                        onInsertKey                 (int row, int col, int pos);
    void                        onRemoveKey                 (int row, int col, int index);
    void                        onUpdateKey                 (int row, int col, int index, float value, float left, float right, uint32_t position);

    void                        setClip                     (const QString &clip);

    void                        onExpanded                  (const QModelIndex &index);
    void                        onCollapsed                 (const QModelIndex &index);

signals:
    void                        changed                     ();

    void                        rowChanged                  ();
    void                        positionChanged             ();

protected:
    AnimationController        *m_pController;

    AnimationStateMachine      *m_pStateMachine;

    AnimationClip              *m_pClip;

    bool                        m_isHighlighted;

    QModelIndex                 m_HoverIndex;

    float                       m_Position;

    int                         m_Row;
    int                         m_Col;

    QMap<QString, AnimationClip *> m_Clips;

    QList<int>                  m_Expands;

};

class UndoInsertKey : public QUndoCommand {
public:
    UndoInsertKey(int row, int col, int pos, AnimationClipModel *model, const QString &name, QUndoCommand *parent = nullptr) :
        QUndoCommand(name, parent),
        m_pModel(model),
        m_Row(row),
        m_Column(col),
        m_Position(pos) {

    }
    void undo() override;
    void redo() override;
protected:
    void insertKey(AnimationCurve &curve);

protected:
    AnimationClipModel *m_pModel;
    int m_Row;
    int m_Column;
    int m_Position;
    list<int> m_Indices;
};

class UndoRemoveKey : public QUndoCommand {
public:
    UndoRemoveKey(int row, int col, int index, AnimationClipModel *model, const QString &name, QUndoCommand *parent = nullptr) :
        QUndoCommand(name, parent),
        m_pModel(model),
        m_Row(row),
        m_Column(col),
        m_Index(index) {

    }
    void undo() override;
    void redo() override;
protected:
    AnimationClipModel *m_pModel;
    int m_Row;
    int m_Column;
    int m_Index;
    AnimationCurve::KeyFrame m_Key;
};

class UndoUpdateKey : public QUndoCommand {
public:
    UndoUpdateKey(int row, int col, int index, float value, float left, float right, uint32_t position, AnimationClipModel *model, const QString &name, QUndoCommand *parent = nullptr) :
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
    uint32_t m_Position;
    AnimationCurve::KeyFrame m_Key;
};

class UndoUpdateKeys : public QUndoCommand {
public:
    UndoUpdateKeys(int row, const QVariant &data, AnimationClipModel *model, const QString &name, QUndoCommand *parent = nullptr) :
        QUndoCommand(name, parent),
        m_pModel(model),
        m_Row(row),
        m_Data(data) {

    }
    void undo() override;
    void redo() override;
protected:
    AnimationClipModel *m_pModel;
    int m_Row;
    QVariant m_Data;
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
    AnimationClip::TrackList m_Tracks;
};

class UndoUpdateItems : public QUndoCommand {
public:
    UndoUpdateItems(AnimationClip::TrackList &tracks, AnimationClipModel *model, const QString &name, QUndoCommand *parent = nullptr) :
        QUndoCommand(name, parent),
        m_pModel(model),
        m_Tracks(tracks) {

    }
    void undo() override;
    void redo() override;
protected:
    AnimationClipModel *m_pModel;
    AnimationClip::TrackList m_Tracks;
};

#endif // ANIMATIONCLIPMODEL_H
