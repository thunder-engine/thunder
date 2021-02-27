#ifndef PLAYHEAD_H
#define PLAYHEAD_H

#include <QGraphicsWidget>

class Ruler;

class PlayHead : public QGraphicsWidget {
public:
    explicit PlayHead(Ruler *ruler);

    void setTime(float time);

    void setHeight(float value);

    void updatePosition();

private:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;

private:
    Ruler *m_ruler;

    float m_time;

};

#endif // PLAYHEAD_H
