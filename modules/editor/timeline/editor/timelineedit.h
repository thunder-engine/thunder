#ifndef TIMELINEEDIT_H
#define TIMELINEEDIT_H

#include <QMenu>

#include <object.h>

#include <editorgadget.h>

#include <animationcurve.h>

class Animator;
class NativeBehaviour;
class AnimationClipModel;

class AnimationClip;
class UndoCommand;

namespace Ui {
    class TimelineEdit;
}

class TimelineEdit : public EditorGadget {
    Q_OBJECT

public:
    explicit TimelineEdit(QWidget *parent = nullptr);
    ~TimelineEdit();

private slots:
    void onUpdated() override;

    void onObjectsSelected(QList<Object *> objects) override;
    void onItemsSelected(QList<QObject *> objects) override;

    void onObjectsChanged(QList<Object *> objects, const QString property, Variant value) override;

    void onPropertyUpdated(Object *object, const QString property);

protected:
    void saveClip();

    Animator *findController(Object *object);

    void setController(Animator *controller);

    QString pathTo(Object *src, Object *dst);

    void updateClips();

    uint32_t position() const;

private slots:
    void onRebind();

    void onSelectKey(int, int);

    void onRowsSelected(QStringList list);

    void onClipChanged(const QString &clip);

    void onKeyChanged();

    void setPosition(uint32_t position);

    void on_play_clicked();

    void on_previous_clicked();

    void on_begin_clicked();

    void on_next_clicked();

    void on_end_clicked();

    void on_flatKey_clicked();

    void on_breakKey_clicked();

    void on_timeEdit_editingFinished();

private:
    void timerEvent(QTimerEvent *) override;

    void changeEvent(QEvent *event) override;

    Ui::TimelineEdit *ui;

    Animator *m_controller;

    NativeBehaviour *m_armature;

    AnimationClipModel *m_model;

    const UndoCommand *m_lastCommand;

    QMap<QString, AnimationClip *> m_clips;

    QString m_currentClip;

    int32_t m_timerId;

    int32_t m_row;
    int32_t m_ind;
};

#endif // TIMELINEEDIT_H
