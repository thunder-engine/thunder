#include <QPainter>
#include <QPaintEvent>

#include "graphwidget.h"

GraphWidget::GraphWidget(QWidget *parent) :
        QLabel(parent) {

    setMouseTracking(true);

    mTranslate      = Vector3(0.0f, 0.0f, 1.0f);
    bCameraControl  = false;
}

void GraphWidget::draw(QPainter &mPainter, const QRect &r) {
}

void GraphWidget::select(const QPoint &pos) {
}

void GraphWidget::paintEvent( QPaintEvent *pe ) {
    QLabel::paintEvent(pe);

    QPainter mPaint(this);

    draw(mPaint, pe->rect());

    mPaint.end();
}

void GraphWidget::wheelEvent(QWheelEvent *pe ) {
    QLabel::wheelEvent(pe);

    float s         = mTranslate.z;
    float delta     = (float)pe->delta() / 1000.0f;
    mTranslate     += Vector3(0.0f, 0.0f, delta);
    if(mTranslate.z < 0)
        mTranslate.z = s;

    repaint();
}

void GraphWidget::mouseMoveEvent( QMouseEvent *pe ) {
    QLabel::mouseMoveEvent(pe);

    select(pe->pos());

    int pos_x   = pe->pos().x();
    int pos_y   = pe->pos().y();

    if(bCameraControl) {
        mTranslate     += Vector3(pos_x - x, pos_y - y, 0.0f);
    }

    x   = pos_x;
    y   = pos_y;

    repaint();
}

void GraphWidget::mousePressEvent( QMouseEvent *pe ) {
    QLabel::mousePressEvent(pe);

    x   = pe->pos().x();
    y   = pe->pos().y();

    if(pe->button() == Qt::RightButton) {
        bCameraControl  = true;
    }

    repaint();
}

void GraphWidget::mouseReleaseEvent( QMouseEvent *pe ) {
    QLabel::mouseReleaseEvent(pe);

    if(pe->button() == Qt::RightButton) {
        bCameraControl  = false;
    }

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
