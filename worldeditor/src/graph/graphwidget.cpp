#include <QPainter>
#include <QPaintEvent>

#include "graphwidget.h"

GraphWidget::GraphWidget(QWidget *parent) :
        QLabel(parent) {

    setMouseTracking(true);
}

void GraphWidget::draw(QPainter &mPainter, const QRect &r) {
}

void GraphWidget::select(const QPoint &pos) {
}

void GraphWidget::paintEvent( QPaintEvent *pe ) {
    QLabel::paintEvent(pe);

    QPainter paint(this);

    draw(paint, rect());

    paint.end();
}

void GraphWidget::wheelEvent(QWheelEvent *pe ) {
    QLabel::wheelEvent(pe);

    repaint();
}

void GraphWidget::mouseMoveEvent( QMouseEvent *pe ) {
    QLabel::mouseMoveEvent(pe);

    select(pe->pos());

    repaint();
}

void GraphWidget::mousePressEvent( QMouseEvent *pe ) {
    QLabel::mousePressEvent(pe);

    repaint();
}

void GraphWidget::mouseReleaseEvent( QMouseEvent *pe ) {
    QLabel::mouseReleaseEvent(pe);

    repaint();
}

void GraphWidget::keyPressEvent( QKeyEvent *pe ) {
    QLabel::keyPressEvent(pe);

    repaint();
}

void GraphWidget::keyReleaseEvent( QKeyEvent *pe ) {
    QLabel::keyPressEvent(pe);

    repaint();
}
