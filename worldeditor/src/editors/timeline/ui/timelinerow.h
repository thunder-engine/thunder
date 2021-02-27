#ifndef TIMELINEROW_H
#define TIMELINEROW_H

#include "rowitem.h"
#include "keyframe.h"

#include <animationcurve.h>

#define ROW 16
#define TREE_WIDTH 400
#define MAX_VALUE 16000
#define OFFSET 32
#define ICON_SIZE 16

class AnimationTrack;
class Ruler;
class TimelineScene;

class TimelineRow;

class TimelineRow : public RowItem {
    Q_OBJECT

public:
    explicit TimelineRow(TreeRow *row);

    AnimationTrack *track() const;
    void setTrack(AnimationTrack *track, int component = -1);

    void setScene(TimelineScene *scene);

    void moveKey(KeyFrame *key, float time);

    void updateKeys();

    QList<KeyFrame *> onRowPressed(const QPointF &point);

    float onRowDoubleClicked(const QPointF &point);

    QList<KeyFrame> &keys();

    KeyFrame *keyAtPosition(float position, bool selected);

    TreeRow *treeRow() const;

    void fixCurve();

private:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;

    void drawKeys(QPainter *painter, bool flag);

    int type() const override;

private:
    QList<KeyFrame> m_keyframes;

    AnimationTrack *m_track;

    Ruler *m_ruler;

    int m_component;
};

#endif // TIMELINEROW_H
