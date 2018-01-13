#ifndef RESULTVIEW_H
#define RESULTVIEW_H

#include "graph/graphwidget.h"

#include "amath.h"

class ResultView : public GraphWidget {
public:
    ResultView              (QWidget *parent = 0);

    void                    draw                (QPainter &mPainter, const QRect &r);

    void                    setResult           (const QImage &image);

    void                    setCaption          (const QString &string);

protected:
    void                    wheelEvent          ( QWheelEvent *pe );

private:
    QImage                  mImage;

    QRect                   mRect;

    QString                 mCaption;

    QBrush                  mBrush;

    float                   mZoom;
};

#endif // RESULTVIEW_H
