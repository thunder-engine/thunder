#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QLabel>

class GraphWidget : public QLabel {
    Q_OBJECT
public:
    explicit GraphWidget    (QWidget *parent = 0);

    virtual void            draw                (QPainter &mPainter, const QRect &r);
    virtual void            select              (const QPoint &pos);

protected:
    void                    paintEvent          ( QPaintEvent *pe );

    void                    wheelEvent          ( QWheelEvent *pe );
    void                    mouseMoveEvent      ( QMouseEvent *pe );
    void                    mousePressEvent     ( QMouseEvent *pe );
    void                    mouseReleaseEvent   ( QMouseEvent *pe );
    void                    keyPressEvent       ( QKeyEvent *pe );
    void                    keyReleaseEvent     ( QKeyEvent *pe );
};

#endif // GRAPHWIDGET_H
