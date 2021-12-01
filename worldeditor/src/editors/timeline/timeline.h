#ifndef TIMELINE_H
#define TIMELINE_H

#include <QWidget>
#include <QMenu>

#include <object.h>

#include <animationcurve.h>

class Animator;
class AnimationClipModel;

class AnimationClip;

namespace Ui {
    class Timeline;
}

class Timeline : public QWidget {
    Q_OBJECT

public:
    explicit Timeline(QWidget *parent = nullptr);
    ~Timeline();

signals:
    void moved();

    void animated(bool);

    void objectSelected(Object::ObjectList objects);

public slots:
    void onObjectsSelected(Object::ObjectList objects);

    void onObjectsChanged(Object::ObjectList objects, const QString property);

    void onPropertyUpdated(Object *object, const QString property);

    void showBar();

protected:
    void saveClip();

    Animator *findController(Object *object);

    void setController(Animator *controller);

    QString pathTo(Object *src, Object *dst);

    void updateClips();

    uint32_t position() const;

private slots:
    void onModified();

    void onRebind();

    void onSelectKey(int, int, int);

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

private:
    void timerEvent(QTimerEvent *) override;

    void changeEvent(QEvent *event) override;

    Ui::Timeline *ui;

    Animator *m_controller;

    AnimationClipModel *m_model;

    QMap<QString, AnimationClip *> m_clips;

    QString m_currentClip;

    int32_t m_TimerId;

    int32_t m_Row;
    int32_t m_Col;
    int32_t m_Ind;

    bool m_Modified;
};

#endif // TIMELINE_H
