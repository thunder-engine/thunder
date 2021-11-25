#include "timelinerow.h"

#include <QPainter>
#include <QDebug>

#include <animationclip.h>

#include "../timelinescene.h"
#include "ruler.h"
#include "treerow.h"

TimelineRow::TimelineRow(TreeRow *row) :
        RowItem(row),
        m_track(nullptr),
        m_ruler(nullptr),
        m_component(-1) {

    setMinimumHeight(ROW);
    setMaximumHeight(ROW);

    setMinimumWidth(MAX_VALUE);
    setMaximumWidth(MAX_VALUE);
}

AnimationTrack *TimelineRow::track() const {
    return m_track;
}

void TimelineRow::setTrack(AnimationTrack *track, int component) {
    m_track = track;
    m_component = component;

    setMinimumHeight(ROW);
    updateKeys();
}

void TimelineRow::setScene(TimelineScene *scene) {
    if(scene) {
        m_ruler = scene->rulerWidget();
    }
}

void TimelineRow::moveKey(KeyFrame *key, float time) {
    key->setPosition(time * 1000.0f / m_track->duration());
    update();
}

void TimelineRow::updateKeys() {
    m_keyframes.clear();
    if(m_component > -1) {
        auto it = m_track->curves().find(m_component);
        for(auto &key : it->second.m_Keys) {
            KeyFrame k(&key, m_row);
            m_keyframes.push_back(k);
        }
    }
    update();
}

KeyFrame *TimelineRow::keyAtPosition(float position, bool selected) {
    for(auto &it : m_keyframes) {
        if(it.position() == position) {
            if(selected && !it.isSelected()) {
                continue;
            }
            return &it;
        }
    }
    return nullptr;
}

TreeRow *TimelineRow::treeRow() const {
    return m_row;
}

QList<KeyFrame *> TimelineRow::onRowPressed(const QPointF &point) {
    QList<KeyFrame *> result;

    QPoint p = mapFromScene(point).toPoint();
    if(m_component > -1) {
        for(auto &it : m_keyframes) {
            float x = m_track->duration() * it.position() / 1000.0f;
            if(m_ruler) {
                x = m_ruler->timeToScreen(x);
            }

            QRect r(x - 8, 0, ICON_SIZE, ICON_SIZE);
            if(r.contains(p)) {
                result.push_back(&it);
            }
        }
    } else {
        TreeRow *row = treeRow();
        int offset = ROW;
        for(auto &it : row->children()) {
            TimelineRow *item = it->timelineItem();
            auto list = item->onRowPressed(QPointF(point.x(), point.y() + offset));
            if(row->isExpanded()) {
                offset += ROW;
            }
            result.append(list);
        }
    }
    return result;
}

float TimelineRow::onRowDoubleClicked(const QPointF &point) {
    QPoint p = mapFromScene(point).toPoint();
    float time = m_ruler->screenToTime(p.x());

    return time * 1000.0f / (float)m_track->duration();
}

QList<KeyFrame> &TimelineRow::keys() {
    return m_keyframes;
}

void TimelineRow::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    QRectF r = rect();
    r.setHeight(ROW);

    painter->setPen(QColor(54, 54, 54));
    painter->setBrush(treeRow()->isHovered() ? QColor(110, 110, 110, 200) : QColor(96, 96, 96, 200));
    painter->drawRect(r);

    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(2, 119, 189, 32));
    r.setWidth(m_ruler->timeToScreen(m_track->duration() / 1000.0f));
    painter->drawRect(r);

    drawKeys(painter, false);

    drawKeys(painter, true);

    for(auto &it : treeRow()->children()) {
        it->timelineItem()->drawKeys(painter, false);
    }

    for(auto &it : treeRow()->children()) {
        it->timelineItem()->drawKeys(painter, true);
    }
}

int TimelineRow::type() const {
    return RowItem::TimelineItem;
}

void TimelineRow::drawKeys(QPainter *painter, bool flag) {
    static const QPixmap keyNormal(":/Style/styles/dark/icons/key-normal.png");
    static const QPixmap keySelect(":/Style/styles/dark/icons/key-select.png");

    for(auto it : m_keyframes) {
        if(it.isSelected() == flag) {
            float x = m_track->duration() * it.position() / 1000.0f;
            if(m_ruler) {
                x = m_ruler->timeToScreen(x);
            }
            painter->drawPixmap(x - 8, 0, flag ? keySelect : keyNormal);
        }
    }
}
