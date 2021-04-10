#ifndef RESULTVIEW_H
#define RESULTVIEW_H

#include "../graphwidget.h"

#include <amath.h>
#include <resources/mesh.h>

class ResultView : public GraphWidget
{
    Q_OBJECT
public:
    ResultView              (QWidget *parent = 0);
    ~ResultView             ();

    void                    draw                (QPainter &mPainter, const QRect &r);

    QImage                  result              () { return mImg; }
    void                    setResult           (Vector4 *result, int w, int h);

public slots:
    void                    onUpdateResult      (const QRect &rect);

    void                    showUVGrid          (bool visible);

private:
    /// \todo: Temp
    bool                    drawUV;

    Vector4                *pResult;
    uchar                  *pData;

    int                     mWidth;
    int                     mHeight;

    QImage                  mImg;
};

#endif // RESULTVIEW_H
