#ifndef ANIMATIONCLIPMODEL_H
#define ANIMATIONCLIPMODEL_H

#include <QAbstractItemModel>

#include <animationcurve.h>
#include <animationclip.h>

#include "undomanager.h"

class AnimatormationStateMachine;

class AssetConverterSettings;

class AnimationClipModel : public QAbstractItemModel {
    Q_OBJECT

public:
    AnimationClipModel(QObject *parent);

    uint32_t findNear(uint32_t current, bool backward = false);

    QVariant data(const QModelIndex &index, int) const override;

    QVariant headerData(int, Qt::Orientation, int) const override;

    int columnCount(const QModelIndex &) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex parent(const QModelIndex &) const override;

    int rowCount(const QModelIndex &) const override;

    void removeItems(const QModelIndexList &list);

    AnimationCurve::KeyFrame *key(int32_t track, int32_t col, int32_t index);

    AnimationTrack &track(int32_t track);

    QString targetPath(QModelIndex &index) const;

    void updateController();

    AnimationClip *clip() const { return m_clip; }
    void setClip(AnimationClip *clip, Actor *root);

    void commitKey(int row, int col, int index, float value, float left, float right, uint32_t position);

    bool isReadOnly() const;

    void propertyUpdated(Object *object, const QString &path, const QString &property, uint32_t position);

signals:
    void rebind();

protected:
    Actor *m_rootActor;

    AnimationClip *m_clip;

    AssetConverterSettings *m_clipSettings;
};

class UndoAnimationClip : public UndoCommand {
public:
    explicit UndoAnimationClip(AnimationClipModel *model, const QString &text, QUndoCommand *parent = nullptr) :
        UndoCommand(text, model, parent),
        m_model(model) {

    }

protected:
    AnimationClipModel *m_model;
};

class UndoUpdateKey : public UndoAnimationClip {
public:
    UndoUpdateKey(int row, int col, int index, float value, float left, float right, uint32_t position, AnimationClipModel *model, const QString &name, QUndoCommand *parent = nullptr) :
        UndoAnimationClip(model, name, parent),
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
    int m_Row;
    int m_Column;
    int m_Index;
    float m_Value;
    float m_Left;
    float m_Right;
    uint32_t m_Position;
    AnimationCurve::KeyFrame m_Key;
};

class UndoRemoveItems : public UndoAnimationClip {
public:
    UndoRemoveItems(QList<int> rows, AnimationClipModel *model, const QString &name, QUndoCommand *parent = nullptr) :
        UndoAnimationClip(model, name, parent),
        m_Rows(rows) {

    }
    void undo() override;
    void redo() override;

protected:
    QList<int> m_Rows;
    AnimationTrackList m_Tracks;
};

class UndoUpdateItems : public UndoAnimationClip {
public:
    UndoUpdateItems(AnimationTrackList &tracks, AnimationClipModel *model, const QString &name, QUndoCommand *parent = nullptr) :
        UndoAnimationClip(model, name, parent),
        m_Tracks(tracks) {

    }
    void undo() override;
    void redo() override;

protected:
    AnimationTrackList m_Tracks;
};

class UndoInsertKey : public UndoAnimationClip {
public:
    explicit UndoInsertKey(int row, int col, float pos, AnimationClipModel *model, const QString &name, QUndoCommand *parent = nullptr) :
        UndoAnimationClip(model, name, parent),
        m_row(row),
        m_column(col),
        m_position(pos) {

    }
    void undo() override;
    void redo() override;

protected:
    void insertKey(AnimationCurve &curve);

protected:
    int m_row;
    int m_column;
    float m_position;
    QList<int> m_indices;
};

#endif // ANIMATIONCLIPMODEL_H
