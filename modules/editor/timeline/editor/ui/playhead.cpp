#include "playhead.h"

#include <QPainter>

#include "timelinerow.h"
#include "ruler.h"

PlayHead::PlayHead(Ruler *ruler) :
        QGraphicsRectItem(),
        m_ruler(ruler),
        m_time(0.0f) {

    setZValue(99);
    setRect(0, 0, 1, 0);
}

void PlayHead::setTime(float time) {
    time = MAX(time, 0);
    m_time = time;
    updatePosition();
}

void PlayHead::setHeight(float value) {
    setRect(rect().x(), rect().y(), rect().width(), MAX_VALUE);
}

void PlayHead::updatePosition() {
    setX(m_ruler->x() + m_ruler->timeToScreen(m_time));
    update();
}

void PlayHead::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    QColor midColor(198, 40, 40);
    painter->setPen(midColor);

    painter->drawLine(0, 0, 0, MAX_VALUE);
}
