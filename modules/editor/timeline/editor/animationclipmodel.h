#ifndef ANIMATIONCLIPMODEL_H
#define ANIMATIONCLIPMODEL_H

#include <QAbstractItemModel>

#include <animationcurve.h>
#include <animationclip.h>

#include <editor/undostack.h>

#include "timelineedit.h"

class AnimatormationStateMachine;

class AssetConverterSettings;

class AnimationClipModel : public QAbstractItemModel {
    Q_OBJECT

public:
    AnimationClipModel(TimelineEdit *editor);

    uint32_t findNear(uint32_t current, bool backward = false);

    QVariant data(const QModelIndex &index, int) const override;

    QVariant headerData(int, Qt::Orientation, int) const override;

    int columnCount(const QModelIndex &) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex parent(const QModelIndex &) const override;

    int rowCount(const QModelIndex &) const override;

    void removeItems(const QModelIndexList &list);

    AnimationCurve::KeyFrame *key(int32_t track, int32_t index);

    AnimationTrack &track(int32_t track);

    QString targetPath(QModelIndex &index) const;

    void updateController();

    AnimationClip *clip() const { return m_clip; }
    void setClip(AnimationClip *clip, Actor *root);

    void commitKey(int row, int index, float value, float left, float right, uint32_t position);

    bool isReadOnly() const;

    void propertyUpdated(Object *object, const QString &path, const QString &property, uint32_t position);

    UndoStack *undoRedo() const { return m_editor->undoRedo(); }

signals:
    void rebind();

protected:
    Actor *m_rootActor;

    AnimationClip *m_clip;

    AssetConverterSettings *m_clipSettings;

    TimelineEdit *m_editor;

};

class UndoAnimationClip : public UndoCommand {
public:
    explicit UndoAnimationClip(AnimationClipModel *model, const TString &text, UndoCommand *parent = nullptr) :
            UndoCommand(text, parent),
            m_model(model) {

    }

protected:
    AnimationClipModel *m_model;
};

class UndoUpdateKey : public UndoAnimationClip {
public:
    UndoUpdateKey(int row, int index, float value, float left, float right, uint32_t position, AnimationClipModel *model, const TString &text, UndoCommand *parent = nullptr) :
            UndoAnimationClip(model, text, parent),
            m_row(row),
            m_index(index),
            m_value(value),
            m_left(left),
            m_right(right),
            m_position(position) {

    }
    void undo() override;
    void redo() override;

protected:
    AnimationCurve::KeyFrame m_key;

    int m_row;
    int m_index;
    float m_value;
    float m_left;
    float m_right;
    uint32_t m_position;

};

class UndoRemoveItems : public UndoAnimationClip {
public:
    UndoRemoveItems(std::list<int> rows, AnimationClipModel *model, const TString &text, UndoCommand *parent = nullptr) :
            UndoAnimationClip(model, text, parent),
            m_rows(rows) {

    }
    void undo() override;
    void redo() override;

protected:
    std::list<int> m_rows;
    AnimationTrackList m_tracks;
};

class UndoUpdateItems : public UndoAnimationClip {
public:
    UndoUpdateItems(AnimationTrackList &tracks, AnimationClipModel *model, const TString &text, UndoCommand *parent = nullptr) :
            UndoAnimationClip(model, text, parent),
            m_tracks(tracks) {

    }
    void undo() override;
    void redo() override;

protected:
    AnimationTrackList m_tracks;

};

class UndoInsertKey : public UndoAnimationClip {
public:
    explicit UndoInsertKey(int row, float pos, AnimationClipModel *model, const TString &text, UndoCommand *parent = nullptr) :
            UndoAnimationClip(model, text, parent),
            m_row(row),
            m_position(pos) {

    }
    void undo() override;
    void redo() override;

protected:
    void insertKey(AnimationCurve &curve);

protected:
    std::list<int> m_indices;
    int m_row;
    float m_position;

};

#endif // ANIMATIONCLIPMODEL_H
