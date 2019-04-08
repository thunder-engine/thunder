#include <QPainter>
#include <QPaintEvent>
#include <QStaticText>
#include <QMenu>
#include <QImage>
#include <QMimeData>
#include <QWidgetAction>

#include <engine.h>

#include "schemeeditor.h"
#include "config.h"

#include "editors/componentbrowser/componentbrowser.h"

QT_BEGIN_NAMESPACE
  extern Q_WIDGETS_EXPORT void qt_blurImage(QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0 );
QT_END_NAMESPACE

SchemeEditor::SchemeEditor(QWidget *parent) :
        GraphWidget(parent) {

    mFontStride     = gFontSize / 2;
    mFontOffset     = gFontSize * 2;

    mBlurRadius     = 50;

    x               = 0;
    y               = 0;

    mFont           = QFont(gDefaultFont, gFontSize);

    mFontColor      = QColor(0, 0, 0);
    mBorderColor    = QColor(212, 212, 212);
    mFillColor      = QColor(240, 240, 240);

    mTranslate      = QPoint(0, 0);
    mZoom           = 1.0f;

    m_pNode         = nullptr;
    m_pItem         = nullptr;

    m_pFocusNode    = nullptr;
    m_pFocusItem    = nullptr;

    m_pModel        = nullptr;

    m_bCameraControl= false;
    m_bCameraMove   = false;
    drag            = false;

    m_bLinkRemove   = false;

    m_bModified     = false;

    setFocusPolicy(Qt::StrongFocus);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setAcceptDrops(true);

    m_pBrowser  = new ComponentBrowser(this);
    connect(m_pBrowser, SIGNAL(componentSelected(QString)), this, SLOT(onComponentSelected(QString)));

    m_pCreateMenu   = new QMenu(this);
    m_pAction       = new QWidgetAction(m_pCreateMenu);
    m_pAction->setDefaultWidget(m_pBrowser);
    m_pCreateMenu->addAction(m_pAction);
}

void SchemeEditor::init(const QStringList &groups) {
    m_pBrowser->setGroups(groups);
}

void SchemeEditor::draw(QPainter &painter, const QRect &r) {
    mRect   = r;
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QColor(96, 96, 96));
    painter.drawRect(mRect);

    if(m_pModel) {
        painter.setPen(QColor(66, 66, 66));
        painter.setFont(QFont("Arial", 128));

        painter.drawText(48, mRect.height() - 48, m_pModel->metaObject()->className());
        painter.translate(mRect.width() * 0.5, mRect.height() * 0.5);
        painter.translate(mTranslate.x(), mTranslate.y());
        painter.scale(mZoom, mZoom);

        const AbstractSchemeModel::LinkList &links   = m_pModel->getLinks();
        const AbstractSchemeModel::NodeList &nodes   = m_pModel->getNodes();

        for(auto it : links) {
            QColor color(255, 255, 255, 0);

            if(m_pFocusItem == it->oport || m_pFocusItem == it->iport) {
                color   = QColor(255, 255, 0);
            } else {
                color   = QColor(255, 0, 0);
            }

            drawLink(painter, color, it);
        }
        // Draw creation link if exist
        if(m_pNode && m_pItem && !m_bLinkRemove) {
            QPen pen(QColor(255, 255, 0));
            pen.setWidth(2);
            painter.setPen(pen);

            QPoint b((x - mTranslate.x() - mRect.width()  * 0.5) / mZoom,
                     (y - mTranslate.y() - mRect.height() * 0.5) / mZoom);
            QPoint e    = b;

            QRect rect  = calcRect(m_pNode);

            if(m_pItem->out) {
                b   = QPoint(rect.x() + rect.width(), rect.y() + itemPos(m_pItem) + mFontStride);
            } else {
                e   = QPoint(rect.x(), rect.y() + itemPos(m_pItem) + mFontStride);
            }

            int length  = abs(e.x() - b.x()) * 0.5;

            painter.setBrush(Qt::NoBrush);
            QPainterPath path(b);
            path.cubicTo(b.x() + length, b.y(),
                         e.x() - length, e.y(),
                         e.x(), e.y());
            painter.drawPath(path);
        }

        painter.resetTransform();

        for(auto it : nodes) {
            drawNode(painter, it);
        }
    }
}

void SchemeEditor::select(const QPoint &pos) {
    m_pFocusNode    = nullptr;
    m_pFocusItem    = nullptr;

    AbstractSchemeModel::NodeList &nodes = m_pModel->getNodes();
    for(auto it : nodes) {
        hitNode(it, pos);
    }
    if(m_pFocusItem) {
        setCursor(Qt::CrossCursor);
    } else {
        unsetCursor();
    }
}

void SchemeEditor::setModel(AbstractSchemeModel *model) {
    m_pModel    = model;
    m_pBrowser->setModel(m_pModel->components());
    connect(m_pModel, SIGNAL(schemeUpdated()), this, SLOT(repaint()));
    repaint();
}

void SchemeEditor::on_customContextMenuRequested(const QPoint &) {
    if(!m_bCameraMove) {
        m_pCreateMenu->exec(QCursor::pos());
    }
}

void SchemeEditor::onComponentSelected(const QString &path) {
    m_pCreateMenu->hide();
    AbstractSchemeModel::Node *node  = m_pModel->createNode(path);
    if(node) {
        QSize s     = rect().size() / 2;
        node->pos   = QPoint((x - mTranslate.x() - s.width()) / mZoom,
                             (y - mTranslate.y() - s.height()) / mZoom);

        repaint();
    }
}

void SchemeEditor::resizeEvent(QResizeEvent *pe) {
    repaint();
}

void SchemeEditor::wheelEvent(QWheelEvent *pe) {
    GraphWidget::wheelEvent(pe);

    float s = mZoom;
    mZoom  += (float)pe->delta() / 1000.0f;
    if(mZoom < 0) {
        mZoom = s;
    }

    repaint();
}

void SchemeEditor::mouseMoveEvent(QMouseEvent *pe) {
    int pos_x   = pe->pos().x();
    int pos_y   = pe->pos().y();
    if(m_pNode && drag) {
        m_pNode->pos   += QPoint((pos_x - x) / mZoom, (pos_y - y) / mZoom);
    }

    if(m_bCameraControl) {
        m_bCameraMove   = true;
        mTranslate     += QPoint(pos_x - x, pos_y - y);
    }

    x   = pos_x;
    y   = pos_y;

    GraphWidget::mouseMoveEvent(pe);
}

void SchemeEditor::mousePressEvent(QMouseEvent *pe) {
    x   = pe->pos().x();
    y   = pe->pos().y();

    if(pe->button() == Qt::LeftButton) {
        if(m_pFocusNode) {
            m_pNode     = m_pFocusNode;
            emit nodeSelected(m_pNode->ptr);
            if(m_pFocusItem) {
                m_pItem = m_pFocusItem;
            } else {
                drag    = true;
            }
        }
    }

    if(pe->button() == Qt::RightButton) {
        m_bCameraControl= true;
        m_bCameraMove   = false;
    }

    GraphWidget::mousePressEvent(pe);
}

void SchemeEditor::mouseReleaseEvent(QMouseEvent *pe) {
    m_bCameraControl    = false;

    if(pe->button() == Qt::LeftButton) {
        if(m_pNode) {
            if(m_bLinkRemove) {
                if(m_pFocusItem) {
                    m_pModel->deleteLink(m_pFocusItem);
                }
            } else {
                if(m_pItem && m_pFocusItem) {
                    if(m_pItem->out && !m_pFocusItem->out) {
                        m_pModel->createLink(m_pNode, m_pItem, m_pFocusNode, m_pFocusItem);
                    } else if(!m_pItem->out && m_pFocusItem->out) {
                        m_pModel->createLink(m_pFocusNode, m_pFocusItem, m_pNode, m_pItem);
                    }
                }
            }
        }

        drag    = false;
        if(m_pNode != m_pFocusNode) {
            m_pNode = nullptr;
            emit nodeSelected(m_pModel);
        }
        m_pItem = nullptr;
    }

    if(pe->button() == Qt::RightButton) {
        if(!m_bCameraMove) {
            x   = pe->pos().x();
            y   = pe->pos().y();

            on_customContextMenuRequested(pe->pos());
        }
    }
    GraphWidget::mouseReleaseEvent(pe);
}

void SchemeEditor::keyPressEvent(QKeyEvent *pe) {
    GraphWidget::keyPressEvent(pe);

    switch (pe->key()) {
        case Qt::Key_Delete: {
            if(m_pNode && !m_pNode->root) {
                m_pModel->deleteNode(m_pNode);
                emit nodeSelected(m_pModel);
            }
        } break;
        case Qt::Key_Alt: {
            m_bLinkRemove   = true;
        } break;
        default: break;
    }
}

void SchemeEditor::keyReleaseEvent(QKeyEvent *pe) {
    GraphWidget::keyPressEvent(pe);

    switch (pe->key()) {
        case Qt::Key_Alt: {
            m_bLinkRemove   = false;
        } break;
        default: break;
    }
}

void SchemeEditor::dragEnterEvent(QDragEnterEvent *event) {
    if(event->mimeData()->hasFormat(gMimeComponent)) {
        event->acceptProposedAction();
    }
}

void SchemeEditor::dragLeaveEvent(QDragLeaveEvent *event) {
    GraphWidget::dragLeaveEvent(event);
}

void SchemeEditor::dropEvent(QDropEvent *event) {
    x   = event->pos().x();
    y   = event->pos().y();

    onComponentSelected(QString(event->mimeData()->data(gMimeComponent)));

    event->acceptProposedAction();
}

void SchemeEditor::drawLink(QPainter &painter, const QColor &color, const AbstractSchemeModel::Link *link) {
    QPen pen(color);
    pen.setWidth(2);
    painter.setPen(pen);

    QRect rect1 = calcRect(link->sender);
    QRect rect2 = calcRect(link->receiver);

    Vector2 b(rect1.x() + rect1.width(), rect1.y() + itemPos(link->oport) + mFontStride);
    Vector2 e(rect2.x(), rect2.y() + itemPos(link->iport) + mFontStride);

    int lenght  = abs(e.x - b.x) * 0.5;

    painter.setBrush(Qt::NoBrush);
    QPainterPath path(QPointF(b.x, b.y));
    path.cubicTo(b.x + lenght, b.y,
                 e.x - lenght, e.y,
                 e.x, e.y);
    painter.drawPath(path);
}

void SchemeEditor::drawNode(QPainter &painter, const AbstractSchemeModel::Node *node) {
    QRect rect = calcRect(node);

    painter.translate(mRect.width() * 0.5, mRect.height() * 0.5);

    painter.translate(mTranslate.x(), mTranslate.y());
    painter.scale(mZoom, mZoom);

    painter.translate(rect.x(), rect.y());

    painter.drawImage(-mBlurRadius, -mBlurRadius, node->cache);

    if(m_pNode == node) {
        QPen pen(QColor(240, 0, 0));
        pen.setWidth(3);
        painter.setPen(pen);
    } else if(m_pFocusNode == node) {
        QPen pen(QColor(255, 240, 0));
        pen.setWidth(3);
        painter.setPen(pen);
    }  else {
        painter.setPen(mBorderColor);
    }
    painter.setBrush(mFillColor);
    painter.drawRoundedRect(0,  0, rect.width(), rect.height(), gRoundness, gRoundness);
    painter.setPen(mBorderColor);
    painter.drawLine(1, mFontOffset, rect.width() - 1, mFontOffset);

    // Draw title
    painter.setPen(mFontColor);
    mFont.setBold(true);
    painter.setFont(mFont);
    painter.drawText(QRect(0, 0, rect.width(), mFontOffset), Qt::AlignCenter, node->name);
    mFont.setBold(false);

    // Draw properties
    for(auto it : node->list) {
        drawItem(painter, it, rect.size());
    }

    painter.resetTransform();
}

void SchemeEditor::drawItem(QPainter &painter, const AbstractSchemeModel::Item *item, const QSize &size) {
    switch(item->type) {
        case QMetaType::QVector3D:
        case QMetaType::QColor: {
            painter.setBrush(QColor(0, 128, 255));
        } break;
        case QMetaType::UnknownType: {
            painter.setBrush(QColor(0, 0, 0));
        } break;
        case QMetaType::Bool: {
            painter.setBrush(QColor(0, 255, 128));
        } break;
        case QMetaType::Int: {
            painter.setBrush(QColor(255, 0, 128));
        } break;
        case QMetaType::Double: {
            painter.setBrush(QColor(128, 0, 255));
        } break;
        case QMetaType::QString: {
            painter.setBrush(QColor(255, 128, 0));
        } break;
        default: {
            painter.setBrush(QColor(255, 255, 255));
        } break;
    }

    if(m_pFocusItem == item) {
        painter.setBrush(QColor(255, 0, 255));
    }

    painter.setPen(mBorderColor);
    painter.drawEllipse(((item->out) ? size.width() : 0) - mFontStride, itemPos(item), gFontSize, gFontSize);

    painter.setPen(mFontColor);
    painter.setFont(mFont);

    QString name    = item->name;
    if(name[0] >= '0' && name[0] <= '9') {
        name.remove(0, 1);
    }
    painter.drawText(QRectF(gFontSize, itemPos(item) - mFontStride, size.width() - gFontSize * 2, mFontOffset),
                    (item->out) ? Qt::AlignRight : Qt::AlignLeft,
                     name);
}

QRect SchemeEditor::calcRect(const AbstractSchemeModel::Node *node) {
    QRect result;
    result.setTopLeft(node->pos);

    QFontMetrics metrics(mFont);

    result.setWidth(metrics.width(node->name) + mFontOffset);

    int in      = 0;
    int out     = 0;
    int wIn     = 0;
    int wOut    = 0;

    for(auto it : node->list) {
        int width   = metrics.width(it->name);
        if(it->out) {
            wOut    = qMax(wOut, width);
            out++;
        } else {
            wIn     = qMax(wIn, width);
            in++;
        }
    }
    result.setWidth(qMax(result.width(), wIn + wOut + mFontOffset));
    result.setHeight((qMax(in, out) + 1) * mFontOffset);

    return result;
}

void SchemeEditor::hitNode(AbstractSchemeModel::Node *node, const QPoint &pos) {
    QPoint p((node->pos.x() * mZoom + mTranslate.x() + mRect.width() * 0.5f),
             (node->pos.y() * mZoom + mTranslate.y() + mRect.height() * 0.5f) );

    QRect rect  = calcRect(node);

    QRect rc;
    // Check focus on node
    rc  = QRect(p.x(), p.y(), rect.width() * mZoom, rect.height() * mZoom);
    if(rc.contains(pos)) {
        m_pFocusNode    = node;
    }
    // Check focus on properties
    for(auto it : node->list) {
        int x;
        if(it->out) {
            x   = rect.width() * 0.5f + mFontStride;
        } else {
            x   = -mFontStride;
        }
        int y   = itemPos(it);

        rc      = QRect(x * mZoom + p.x(), y * mZoom + p.y(), rect.width() * 0.5f * mZoom, gFontSize * mZoom);
        if(rc.contains(pos)) {
            m_pFocusNode    = node;
            m_pFocusItem    = it;
            return;
        }
    }
}

int SchemeEditor::itemPos(const AbstractSchemeModel::Item *item) {
    return (mFontOffset * 1.25f) + item->pos * mFontOffset;
}
