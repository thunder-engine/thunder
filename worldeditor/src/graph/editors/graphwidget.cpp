#include <QPainter>
#include <QPaintEvent>

#include "graphwidget.h"

GraphWidget::GraphWidget(QWidget *parent) :
        QWidget(parent) {

    setMouseTracking(true);
}

void GraphWidget::draw(QPainter &, const QRect &) {
}

void GraphWidget::select(const QPoint &) {
}

void GraphWidget::paintEvent( QPaintEvent *pe ) {
    QWidget::paintEvent(pe);

    QPainter paint(this);

    draw(paint, rect());

    paint.end();
}

void GraphWidget::wheelEvent(QWheelEvent *pe ) {
    QWidget::wheelEvent(pe);

    repaint();
}

void GraphWidget::mouseMoveEvent( QMouseEvent *pe ) {
    QWidget::mouseMoveEvent(pe);

    select(pe->pos());

    repaint();
}

void GraphWidget::mousePressEvent( QMouseEvent *pe ) {
    QWidget::mousePressEvent(pe);

    repaint();
}

void GraphWidget::mouseReleaseEvent( QMouseEvent *pe ) {
    QWidget::mouseReleaseEvent(pe);

    repaint();
}

void GraphWidget::keyPressEvent( QKeyEvent *pe ) {
    QWidget::keyPressEvent(pe);

    repaint();
}

void GraphWidget::keyReleaseEvent( QKeyEvent *pe ) {
    QWidget::keyPressEvent(pe);

    repaint();
}
