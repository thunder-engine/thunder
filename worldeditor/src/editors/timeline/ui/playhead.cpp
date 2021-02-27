#include "playhead.h"

#include <QPainter>

#include "timelinerow.h"
#include "ruler.h"

PlayHead::PlayHead(Ruler *ruler) :
        QGraphicsWidget(),
        m_ruler(ruler),
        m_time(0.0f) {

    setMinimumWidth(1);
    setMaximumWidth(1);
}

void PlayHead::setTime(float time) {
    time = MAX(time, 0);
    m_time = time;
    updatePosition();
}

void PlayHead::setHeight(float value) {
    setMinimumHeight(value);
    setMaximumHeight(value);
}

void PlayHead::updatePosition() {
    setX(m_ruler->x() + m_ruler->timeToScreen(m_time));
}

void PlayHead::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    QColor midColor(198, 40, 40);
    painter->setPen(midColor);

    painter->drawLine(0, 0, 0, MAX_VALUE);
}
