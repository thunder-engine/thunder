#include "treerow.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsLinearLayout>
#include <QDebug>

#include "../timelinescene.h"

TreeRow::TreeRow(TimelineScene *scene, TreeRow *parent) :
        RowItem(nullptr),
        m_scene(scene),
        m_parent(parent),
        m_timeline(this),
        m_arrowRect(QRect(OFFSET, 0, ICON_SIZE, ICON_SIZE)),
        m_expanded(false),
        m_hover(false) {

    setMinimumHeight(ROW);
    setMaximumHeight(ROW);

    m_label.setParentItem(this);
    m_label.setDefaultTextColor(QColor("#FFFFFF"));
    m_label.setPos(OFFSET + ICON_SIZE, -1);

    m_timeline.setPos(OFFSET, 0);

    m_timeline.setScene(m_scene);

    setFlags(QGraphicsItem::ItemIsSelectable);
}

QModelIndex TreeRow::index() const {
    return m_index;
}

void TreeRow::setName(const QString &name) {
    m_label.setPlainText(name);
}

void TreeRow::setTrack(AnimationTrack *track, const QModelIndex &index) {
    m_index = index;
    if(m_index.parent().isValid()) {
        m_timeline.setTrack(track, m_index.row());
    } else {
        m_timeline.setTrack(track, -1);
    }
}

void TreeRow::insertToLayout(int position) {
    m_scene->treeLayout()->insertItem(position, this);
    m_scene->timelineLayout()->insertItem(position, &m_timeline);
}

void TreeRow::onRowPressed(const QPointF &point) {
    QPoint p = mapFromScene(point).toPoint();

    if(!m_children.isEmpty() && m_arrowRect.contains(p)) {
        m_expanded = !m_expanded;
        for(auto it : m_children) {
            it->setVisible(m_expanded);
            it->timelineItem()->setVisible(m_expanded);
        }
        update();
        m_timeline.update();
    }
}

TimelineRow *TreeRow::timelineItem() {
    return &m_timeline;
}

void TreeRow::addChild(TreeRow *child) {
    m_children.append(child);
    child->setVisible(m_expanded);
    child->m_timeline.setVisible(m_expanded);
}

QList<TreeRow *> &TreeRow::children() {
    return m_children;
}

TreeRow *TreeRow::parentRow() const {
    return m_parent;
}

void TreeRow::mouseHover(bool flag) {
    if(m_hover != flag) {
        m_hover = flag;
        update();
        m_timeline.update();
    }
}

bool TreeRow::isExpanded() const {
    return m_expanded;
}

bool TreeRow::isHovered() const {
    return m_hover;
}

void TreeRow::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    QRectF r = rect();
    painter->setPen(QColor(54, 54, 54));
    painter->setBrush(isSelected() ? QColor(2, 119, 189, 200) : (m_hover ? QColor(110, 110, 110, 200) : QColor(96, 96, 96, 200)));
    r.setWidth(TREE_WIDTH + OFFSET);
    painter->drawRect(r);

    static const QPixmap pixDown (":/Style/styles/dark/icons/arrow-down.png");
    static const QPixmap pixRight(":/Style/styles/dark/icons/arrow-right.png");

    if(!m_children.isEmpty()) {
        painter->drawPixmap(m_arrowRect, (m_expanded) ? pixDown : pixRight);
    }
}

int TreeRow::type() const {
    return RowItem::TreeItem;
}
