#ifndef COMPONENTEDITOR_H
#define COMPONENTEDITOR_H

#include "../graphwidget.h"

#include <aobject.h>

class Engine;

class ComponentEditor : public GraphWidget {
    Q_OBJECT
public:
    explicit ComponentEditor(QWidget *parent = 0);

    void                    init                (Engine *engine);

    void                    draw                (QPainter &painter, const QRect &r);
    void                    select              (const QPoint &pos);

    void                    setObject           (AObject &object);

    void                    setModified         (bool value) { m_bModified = value; }
    bool                    isModified          () { return m_bModified; }

    void                    setUnique           (bool value) { m_bUnique = value; }
    bool                    isUnique            () { return m_bUnique; }

    void                    createComponent     (const QString &uri, AObject *parent = 0);
    void                    deleteComponent     (AObject &object);

    void                    initObject          ();

signals:
    void                    nodeSelected        (AObject *object);
    void                    nodeDeleted         ();

public slots:
    void                    mouseMoveEvent      ( QMouseEvent *pe );
    void                    mousePressEvent     ( QMouseEvent *pe );
    void                    mouseReleaseEvent   ( QMouseEvent *pe );

    void                    keyPressEvent       ( QKeyEvent *pe );

    void                    dragEnterEvent      (QDragEnterEvent *event);
    void                    dragMoveEvent       (QDragMoveEvent *event);
    void                    dropEvent           (QDropEvent *event);

private:
    void                    drawComponent       (QPainter &painter, const QRect &r, AObject *object);

    bool                    m_bModified;
    bool                    m_bUnique;

    int                     mFontSize;

    int                     mBlurRadius;

    QFont                   mFont;

    QColor                  mFontColor;
    QColor                  mBorderColor;
    QColor                  mFillColor;

    QList<QImage>           mCache;

    QImage                  mDisable;
    QImage                  mEnable;
    QImage                  mGraph;

    Engine                *m_pEngine;

    AObject                *m_pObject;
    AObject                *m_pFocused;
    AObject                *m_pSelected;
};

#endif // COMPONENTEDITOR_H
