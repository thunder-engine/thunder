#ifndef TIMELINECONTROLLER_H
#define TIMELINECONTROLLER_H

#include <animationcurve.h>
#include <resources/animationclip.h>

#include <editor/undostack.h>

#include "timelineedit.h"

class AssetConverterSettings;

class TimelineController : public QObject {
    Q_OBJECT

public:
    TimelineController(TimelineEdit *editor);

    uint32_t findNear(uint32_t current, bool backward = false);

    void removeItems(const std::list<int> &rows);

    AnimationCurve::KeyFrame *key(int32_t track, int32_t index);

    AnimationClip *clip() const { return m_clip; }
    void setClip(AnimationClip *clip);

    Actor *root() const { return m_rootActor; }
    void setRoot(Actor *root);

    void commitKey(int row, int index, float value, float left, float right, uint32_t position);

    bool isReadOnly() const;

    void propertyUpdated(Object *object, const QString &path, const QString &property, uint32_t position);

    UndoStack *undoRedo() const { return m_editor->undoRedo(); }

signals:
    void rebind();
    void updated();

protected:
    Actor *m_rootActor;

    AnimationClip *m_clip;

    AssetConverterSettings *m_clipSettings;

    TimelineEdit *m_editor;

};

class UndoAnimationClip : public UndoCommand {
public:
    explicit UndoAnimationClip(TimelineController *controller, const TString &text, UndoCommand *parent = nullptr) :
            UndoCommand(text, parent),
            m_controller(controller) {

    }

protected:
    TimelineController *m_controller;
};

class UndoUpdateKey : public UndoAnimationClip {
public:
    UndoUpdateKey(int row, int index, const std::vector<float> &value, const std::vector<float> &left, const std::vector<float> &right, uint32_t position, TimelineController *controller, const TString &text, UndoCommand *parent = nullptr) :
            UndoAnimationClip(controller, text, parent),
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
    std::vector<float> m_value;
    std::vector<float> m_left;
    std::vector<float> m_right;
    uint32_t m_position;

};

class UndoRemoveItems : public UndoAnimationClip {
public:
    UndoRemoveItems(std::list<int> rows, TimelineController *controller, const TString &text, UndoCommand *parent = nullptr) :
            UndoAnimationClip(controller, text, parent),
            m_rows(rows) {

    }
    void undo() override;
    void redo() override;

protected:
    std::list<int> m_rows;
    AnimationTracks m_tracks;
};

class UndoUpdateItems : public UndoAnimationClip {
public:
    UndoUpdateItems(AnimationTracks &tracks, TimelineController *controller, const TString &text, UndoCommand *parent = nullptr) :
            UndoAnimationClip(controller, text, parent),
            m_tracks(tracks) {

    }
    void undo() override;
    void redo() override;

protected:
    AnimationTracks m_tracks;

};

class UndoInsertKey : public UndoAnimationClip {
public:
    explicit UndoInsertKey(int row, float pos, TimelineController *controller, const TString &text, UndoCommand *parent = nullptr) :
            UndoAnimationClip(controller, text, parent),
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

#endif // TIMELINECONTROLLER_H
