#include "ruler.h"

#include <QPainter>
#include <QWidget>
#include <QDebug>

#include "timelinerow.h"

#define MIN_STEP 3
#define MAX_STEP 30

Ruler::Ruler() :
        RowItem(nullptr),
        m_Step(MIN_STEP * 2),
        m_Scale(0.01f),
        m_TranslateX(0),
        m_maxDuration(0) {

    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
}

float Ruler::timeToScreen(float time) {
    return time * m_Step / m_Scale;
}

float Ruler::screenToTime(float screen, bool snap) {
    float v = screen / m_Step;
    if(snap) {
        v = roundf(v);
    }
    return v * m_Scale;
}

int32_t Ruler::maxDuration() const {
    return m_maxDuration;
}

void Ruler::setMaxDuration(int32_t duration) {
    if(m_maxDuration != duration) {
        m_maxDuration = duration;

        emit maxDurationChanged();
    }
}

void Ruler::zoomIn() {
    m_Step++;
    if(m_Step > MAX_STEP) {
        if(m_Scale > 0.001f) {
            m_Step = MIN_STEP;
            m_Scale /= 10.0f;
        } else {
            m_Step--;
            return;
        }
    }
    emit zoomChanged();
    update();
}

void Ruler::zoomOut() {
    m_Step--;
    if(m_Step < MIN_STEP) {
        m_Step = MAX_STEP;
        m_Scale *= 10.0f;
    }
    emit zoomChanged();
    update();
}

void Ruler::setTranslateX(int value) {
    if(m_TranslateX != value) {
        m_TranslateX = value;
        update();
    }
}

void Ruler::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *widget) {
    QFont font = painter->font();
    font.setPointSize(8);
    painter->setFont(font);

    int beforeX = MAX(round(m_TranslateX / m_Step), 0);
    int afterX = round((widget->width() + m_TranslateX) / m_Step) + 1;

    int maxDuration = timeToScreen(m_maxDuration / 1000.0f);

    int midAlpha = (m_Step / (float)MAX_STEP) * 255;
    QColor midColor(54, 54, 54, midAlpha);
    painter->setPen(midColor);
    for(int i = beforeX; i < afterX; i++) {
        int x = i * m_Step;

        int h = 12;
        if(i % 10 == 0) {
            h = 5;
            painter->setPen((x < maxDuration) ? QColor(54, 54, 54) : QColor(54, 54, 54, 128));
            painter->drawText(QRectF(x + 2, -3, 30, 16), Qt::AlignVCenter, QString::number(i * m_Scale));
            painter->drawLine(x, h, x, MAX_VALUE);
            painter->setPen(midColor);
        } else {
            painter->drawLine(x, h, x, MAX_VALUE);
        }
    }
}

int Ruler::type() const {
    return RowItem::RulerItem;
}
