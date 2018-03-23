#ifndef COMPONENTEDITOR_H
#define COMPONENTEDITOR_H

#include "../graphwidget.h"

#include <object.h>

class Engine;

class ComponentEditor : public GraphWidget {
    Q_OBJECT
public:
    explicit ComponentEditor(QWidget *parent = 0);

    void                    init                (Engine *engine);

    void                    draw                (QPainter &painter, const QRect &r);
    void                    select              (const QPoint &pos);

    void                    setObject           (Object &object);

    void                    setModified         (bool value) { m_bModified = value; }
    bool                    isModified          () { return m_bModified; }

    void                    setUnique           (bool value) { m_bUnique = value; }
    bool                    isUnique            () { return m_bUnique; }

    void                    createComponent     (const QString &uri, Object *parent = 0);
    void                    deleteComponent     (Object &object);

    void                    initObject          ();

signals:
    void                    nodeSelected        (Object *object);
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
    void                    drawComponent       (QPainter &painter, const QRect &r, Object *object);

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

    Engine                 *m_pEngine;

    Object                 *m_pObject;
    Object                 *m_pFocused;
    Object                 *m_pSelected;
};

#endif // COMPONENTEDITOR_H
