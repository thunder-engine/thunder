#ifndef CURVEEDITOR_H
#define CURVEEDITOR_H

#include <cstdint>
#include <vector>

#include "graphwidget.h"

#include <amath.h>

enum control_types {
    CONTROL_NONE        = 0,
    CONTROL_POINT       = (1<<1),
    CONTROL_CAMERA      = (1<<2)
};

class CurveEditor : public GraphWidget {
public:
    CurveEditor             (QWidget *parent = 0);

    void                    draw                (QPainter &mPainter, const QRect &r);
    void                    select              (const QPoint &pos);

//    void                    set_curve           (ACurve *value);

    void                    update              ();

    QPoint                 &translate           ();
    void                    setTranslate        (QPoint &value);

    QPointF                &step                ();
    void                    setStep             (QPointF &value);

public slots:
    void                    resizeEvent         (QResizeEvent *pe);

    void                    wheelEvent          ( QWheelEvent *pe );
    void                    mouseMoveEvent      ( QMouseEvent *pe );
    void                    mousePressEvent     ( QMouseEvent *pe );
    void                    mouseReleaseEvent   ( QMouseEvent *pe );
    void                    keyPressEvent       ( QKeyEvent *pe );
    void                    keyReleaseEvent     ( QKeyEvent *pe );

private:
    char                    control;

    bool                    add;
    bool                    remove;

    int                     focus_p;
    int                     focus_v;

    int                     select_p;
    int                     select_v;

    uint32_t                cell;

    QPointF                 m_Step;

    //ACurve                 *m_pCurve;

    QPoint                  m_Translate;

    QPoint                  m_Pos;

    QSize                   m_Size;

    QColor                  m_Color;

    QPainterPath            m_Path;
};

#endif // CURVEEDITOR_H
