#ifndef RULER_H
#define RULER_H

#include "rowitem.h"

class Ruler : public RowItem {
    Q_OBJECT
public:
    Ruler();

    void zoomIn();
    void zoomOut();

    void setTranslateX(int value);

    float timeToScreen(float time);
    float screenToTime(float screen, bool snap = false);

    int32_t maxDuration() const;
    void setMaxDuration(int32_t);

signals:
    void zoomChanged();
    void maxDurationChanged();

private:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;

    int type() const override;

private:
    int m_Step;
    float m_Scale;

    int32_t m_TranslateX;
    int32_t m_maxDuration;
};

#endif // RULER_H
