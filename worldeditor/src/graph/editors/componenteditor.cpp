#include "componenteditor.h"

#include <QPainter>
#include <QMouseEvent>
#include <QMimeData>
#include <QMessageBox>

#include <engine.h>

QT_BEGIN_NAMESPACE
  extern Q_WIDGETS_EXPORT void qt_blurImage(QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0 );
QT_END_NAMESPACE

ComponentEditor::ComponentEditor(QWidget *parent) :
        GraphWidget(parent) {

    m_pEngine       = 0;

    m_bModified     = false;
    m_bUnique       = false;

    mFontSize       = 12;

    mBlurRadius     = 50;

    mFont           = QFont("Helvetica Neue", mFontSize);

    mFontColor      = QColor(0, 0, 0);
    mBorderColor    = QColor(212, 212, 212);
    mFillColor      = QColor(240, 240, 240);

    m_pObject       = 0;
    m_pFocused      = 0;
    m_pSelected     = 0;

    setFocusPolicy(Qt::StrongFocus);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setAcceptDrops(true);

    mDisable.load(QString::fromUtf8(":/Images/Disable.png"));
    mEnable.load(QString::fromUtf8(":/Images/Enable.png"));
    mGraph.load(QString::fromUtf8(":/Images/Graph.png"));
}

void ComponentEditor::init(Engine *engine) {
    m_pEngine   = engine;
}

void ComponentEditor::draw(QPainter &painter, const QRect &r) {
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QColor(96, 96, 96));
    painter.drawRect(r);

    if(m_pObject) {
        uint32_t x  = 0;
        auto i  = mCache.begin();
        for(const auto &it : m_pObject->getChildren()) {
            QRect rect(x * 285 + 5, 5, 275, 70);
            painter.drawImage(rect.x() - mBlurRadius, rect.y() - mBlurRadius, *i );
            drawComponent(painter, rect, it);
            x++;
			++i;
        }
    }
}

void ComponentEditor::select(const QPoint &pos) {
    if(m_pObject) {
        uint32_t x  = 0;
        m_pFocused      = 0;
        for(const auto &it : m_pObject->getChildren()) {
            if(QRect(x * 285 + 5, 5, 275, 70).contains(pos)) {
                m_pFocused  = it;
            }
            uint32_t y  = 1;
            for(const auto &component : it->getChildren()) {
                if(QRect(x * 285 + 5, y * 50 + 5, 275, 50).contains(pos)) {
                    m_pFocused  = component;
                }
                y++;
            }
            x++;
        }
    }
}

void ComponentEditor::setObject(Object &object) {
    m_pObject   = &object;

    initObject();
    repaint();
}

void ComponentEditor::createComponent(const QString &uri, Object *parent) {
    bool create = true;
    if(isUnique()) {
        for(const auto &it : parent->getChildren()) {
            if(it->typeName() == uri.toStdString()) {
                create = false;
                break;
            }
        }
    }

    if(create) {
        Object *component   = m_pEngine->objectCreate(uri.toStdString(), "", parent);
        if(component) {
            initObject();
            repaint();

            m_bModified = true;
        }
    }
}

void ComponentEditor::deleteComponent(Object &object) {
    delete &object;

    initObject();
    repaint();

    m_bModified = true;
}

void ComponentEditor::mouseMoveEvent   ( QMouseEvent *pe ) {
    select(pe->pos());

    repaint();
}

void ComponentEditor::mousePressEvent  ( QMouseEvent *pe ) {
    GraphWidget::mousePressEvent(pe);
    if(pe->button() == Qt::LeftButton) {
        m_pSelected = 0;
        if(m_pFocused) {
            m_pSelected = m_pFocused;
            emit nodeSelected(m_pSelected);
        }
        repaint();
    }
}

void ComponentEditor::mouseReleaseEvent( QMouseEvent *pe ) {
    GraphWidget::mouseReleaseEvent(pe);
}

void ComponentEditor::keyPressEvent(QKeyEvent *pe) {
    GraphWidget::keyPressEvent(pe);

    switch (pe->key()) {
        case Qt::Key_Delete: {
            if(m_pSelected) {
                deleteComponent(*m_pSelected);
                m_pSelected = 0;
                emit nodeDeleted();
            }
        } break;
        default: break;
    }
}

void ComponentEditor::dragEnterEvent(QDragEnterEvent *event) {
    if(event->mimeData()->hasFormat("text/plain")) {
        event->acceptProposedAction();
    }
}

void ComponentEditor::dragMoveEvent(QDragMoveEvent *event) {
    m_pFocused      = 0;
    uint32_t i  = 0;
    for(const auto &it : m_pObject->getChildren()) {
        if(QRect(i * 275, 0, 275, height()).contains(event->pos())) {
            m_pFocused  = it;
        }

        i++;
    }
    repaint();
}

void ComponentEditor::dropEvent(QDropEvent *event) {
    createComponent(event->mimeData()->text().split('/').last(), m_pFocused);

    m_pFocused    = 0;
    repaint();

    event->acceptProposedAction();
}

void ComponentEditor::initObject() {
    if(m_pObject) {
        mCache.clear();
        for(const auto &it : m_pObject->getChildren()) {
            Object *components  = it;
            QSize rect(275, 70 + components->getChildren().size() * 50);
            QSize size(rect.width() + 2 * mBlurRadius, rect.height() + 2 * mBlurRadius);

            QImage cache(size, QImage::Format_ARGB32_Premultiplied);
            cache.fill(0);
            QPainter tmpPainter(&cache);
            tmpPainter.setCompositionMode(QPainter::CompositionMode_Source);
            tmpPainter.setBrush(QColor(0, 0, 0));
            tmpPainter.drawRoundedRect(mBlurRadius, mBlurRadius, rect.width(), rect.height(), 8, 8);
            tmpPainter.end();

            // blur the alpha channel
            QImage blurred(cache.size(), QImage::Format_ARGB32_Premultiplied);
            blurred.fill(0);
            QPainter blurPainter(&blurred);
            qt_blurImage(&blurPainter, cache, mBlurRadius, true, true);
            blurPainter.end();

            mCache.push_back(blurred);
        }
    }
}
/// \todo: Replace magic numbers
void ComponentEditor::drawComponent(QPainter &painter, const QRect &r, Object *object) {
    auto components = object->getChildren();
    QRect root = r;
    root.setHeight(root.height() + components.size() * 50);

    painter.setPen(Qt::NoPen);
    painter.setBrush(mFillColor);
    painter.drawRoundedRect(root, 8, 8);

    QRect text = r;
    text.translate(8, 0);

    painter.setPen(mFontColor);
    mFont.setBold(true);
    painter.setFont(mFont);
    painter.drawText(text, Qt::AlignLeft | Qt::AlignVCenter, object->typeName().c_str());
    mFont.setBold(true);

    text.translate(8, 0);
    text.setHeight(50);
    for(const auto &it : object->getChildren()) {
        text.translate(0, 50);
        QRect back  = text;
        back.translate(-16, 0);
        Object *component   = it;
        if(m_pSelected == component) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(255, 0, 0));
            painter.drawRect(back);
        } else if(m_pFocused == component) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(255, 240, 0));
            painter.drawRect(back);
        }

        painter.setPen(mFontColor);
        mFont.setBold(true);
        painter.setFont(mFont);
        painter.drawText(text, Qt::AlignLeft | Qt::AlignVCenter, component->typeName().c_str());
        mFont.setBold(true);
    }

    QPen pen;
    if(m_pSelected == object) {
        pen.setColor(QColor(255, 0, 0));
        pen.setWidth(3);
    } else if(m_pFocused == object) {
        pen.setColor(QColor(255, 240, 0));
        pen.setWidth(3);
    } else {
        pen.setColor(mBorderColor);
        pen.setWidth(1);
    }

    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(root, 8, 8);

}
