#ifndef SCHEMEEDITOR_H
#define SCHEMEEDITOR_H

#include "../../graph/editors/graphwidget.h"

#include "abstractschememodel.h"

class QMenu;

#define SCREEN "_Screen"

class ComponentBrowser;
class QWidgetAction;

class SchemeEditor : public GraphWidget {
    Q_OBJECT
public:
    explicit SchemeEditor   (QWidget *parent = 0);

    void                    init                (const QStringList &groups = QStringList());

    void                    draw                (QPainter &painter, const QRect &r);
    void                    select              (const QPoint &pos);

    void                    setModel            (AbstractSchemeModel *model);

signals:
    void                    schemeUpdated       ();
    void                    nodeSelected        (void *object);

protected:
    void                    resizeEvent         (QResizeEvent *pe);

    void                    wheelEvent          (QWheelEvent *pe);
    void                    mouseMoveEvent      (QMouseEvent *pe);
    void                    mousePressEvent     (QMouseEvent *pe);
    void                    mouseReleaseEvent   (QMouseEvent *pe);

    void                    keyPressEvent       (QKeyEvent *pe);
    void                    keyReleaseEvent     (QKeyEvent *pe);

    void                    dragEnterEvent      (QDragEnterEvent *);
    void                    dragLeaveEvent      (QDragLeaveEvent *);
    void                    dropEvent           (QDropEvent *);

    void                    drawNode            (QPainter &painter, const AbstractSchemeModel::Node *node);
    void                    drawItem            (QPainter &painter, const AbstractSchemeModel::Item *item, const QSize &size);
    void                    drawLink            (QPainter &painter, const QColor &color, const AbstractSchemeModel::Link *link);

    QRect                   calcRect            (const AbstractSchemeModel::Node *node);

    void                    hitNode             (AbstractSchemeModel::Node *node, const QPoint &pos);
    int                     itemPos             (const AbstractSchemeModel::Item *item);

protected slots:
    void                    on_customContextMenuRequested   (const QPoint &pos);

    void                    onComponentSelected             (const QString &path);

private:
    int                     mFontStride;
    int                     mFontOffset;

    int                     mBlurRadius;

    QRect                   mRect;

    int                     x;
    int                     y;

    bool                    drag;
    bool                    m_bCameraControl;
    bool                    m_bCameraMove;
    bool                    m_bLinkRemove;
    bool                    m_bModified;

    QFont                   mFont;
    QColor                  mFontColor;
    QColor                  mBorderColor;
    QColor                  mFillColor;

    QPoint                  mTranslate;
    float                   mZoom;

    QMenu                  *m_pCreateMenu;
    QWidgetAction          *m_pAction;

    ComponentBrowser       *m_pBrowser;

    AbstractSchemeModel    *m_pModel;

    AbstractSchemeModel::Node   *m_pNode;
    AbstractSchemeModel::Item   *m_pItem;

    AbstractSchemeModel::Node   *m_pFocusNode;
    AbstractSchemeModel::Item   *m_pFocusItem;
};

#endif // ASCHEMEEDITOR_H
