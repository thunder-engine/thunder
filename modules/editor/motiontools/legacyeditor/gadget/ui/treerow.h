#ifndef TREEROW_H
#define TREEROW_H

#include "rowitem.h"

#include "timelinerow.h"

class AnimationTrack;
class TimelineScene;

class TreeRow : public RowItem {
    Q_OBJECT
public:
    explicit TreeRow(TimelineScene *scene, TreeRow *parent);

    int row() const;

    void setName(const QString &name);

    void setTrack(AnimationTrack *track, int row);

    void onRowPressed(const QPointF &point);

    TimelineRow *timelineItem();

    void addChild(TreeRow *child);

    QList<TreeRow *> &children();

    TreeRow *parentRow() const;

    void mouseHover(bool flag);

    bool isExpanded() const;
    bool isHovered() const;

    void insertToLayout(int position);

signals:
    void updateScene();

private:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;

    int type() const override;

private:
    TimelineScene *m_scene;
    TreeRow *m_parent;
    TimelineRow m_timeline;
    QGraphicsTextItem m_label;

    QList<TreeRow *> m_children;

    QRect m_arrowRect;

    int m_row;

    bool m_expanded;
    bool m_hover;
};

#endif // TREEROW_H
