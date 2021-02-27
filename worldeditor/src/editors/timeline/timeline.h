#ifndef TIMELINE_H
#define TIMELINE_H

#include <QWidget>
#include <QMenu>

#include <object.h>

#include <animationcurve.h>

class AnimationController;
class AnimationClipModel;

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

public slots:
    void onObjectSelected(Object::ObjectList objects);

    void onUpdated(Object *object, const QString property);

    void onChanged(Object::ObjectList objects, const QString property);

    void showBar();

protected:
    void saveClip();

    AnimationController *findController(Object *object);

    float findNear(bool backward = false);

    QString pathTo(Object *src, Object *dst);

    void updateClips();

private slots:
    void onModified();

    void onSelectKey(int, int, int);

    void onKeyChanged();

    void on_play_clicked();

    void on_previous_clicked();

    void on_begin_clicked();

    void on_next_clicked();

    void on_end_clicked();

    void on_flatKey_clicked();

    void on_breakKey_clicked();

    void on_deleteKey_clicked();

private:
    void timerEvent(QTimerEvent *) override;

    void changeEvent(QEvent *event) override;

    Object::ObjectList m_SelectedObjects;

    Ui::Timeline *ui;

    AnimationController *m_pController;

    AnimationClipModel *m_pModel;

    int32_t m_TimerId;

    int32_t m_Row;
    int32_t m_Col;
    int32_t m_Ind;

    bool m_Modified;
};

#endif // TIMELINE_H
