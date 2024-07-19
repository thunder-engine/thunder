#ifndef TIMELINESCENE_H
#define TIMELINESCENE_H

#include <QGraphicsScene>
#include <QModelIndexList>

class QGraphicsLinearLayout;
class Ruler;
class KeyFrame;
class PlayHead;
class TreeRow;

class AnimationClipModel;

class TimelineScene : public QGraphicsScene {
    Q_OBJECT
public:
    explicit TimelineScene(QWidget *editor);

    QGraphicsLinearLayout *treeLayout() const;
    QGraphicsLinearLayout *timelineLayout() const;

    QGraphicsWidget *rootWidget() const;

    Ruler *rulerWidget() const;

    PlayHead *playHead() const;

    QModelIndexList selectedIndexes() const;

    QList<KeyFrame *> selectedKeyframes();

    void clearSelection();

    TreeRow *row(int row);

    void updateMaxDuration();

    AnimationClipModel *model() const { return m_model; }
    void setModel(AnimationClipModel *model);

signals:
    void headPositionChanged(uint32_t value);
    void rowSelectionChanged();
    void keySelectionChanged(int row, int col, int index);
    void keyPositionChanged(float delta);
    void insertKeyframe(int row, int col, float position);
    void deleteSelectedKey();
    void removeSelectedProperty();

public slots:
    void onPositionChanged(uint32_t time);
    void onContentWidthChanged();

private:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    void wheelEvent(QGraphicsSceneWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

private:
    QGraphicsLinearLayout *m_layoutRoot;
    QGraphicsLinearLayout *m_layoutTree;
    QGraphicsLinearLayout *m_layoutTimeline;

    AnimationClipModel *m_model;

    QGraphicsWidget *m_widgetRoot;
    Ruler *m_rulerItem;
    PlayHead *m_playHead;

    KeyFrame *m_pressedKeyframe;

    QModelIndexList m_selectedRows;

    QPointF m_pressPos;
    float m_pressKeyPosition;
    bool m_drag;
};

#endif // TIMELINESCENE_H
