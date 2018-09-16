#include "timelineeditor.h"

#include <QPainter>
#include <QWheelEvent>
#include <QDebug>

#include "config.h"

#include <components/animationcontroller.h>

#include <amath.h>

#define HEIGHT      17
#define HEADER      19
#define FONT_SIZE   8
#define MIN_STEP    8
#define MAX_STEP    40
#define ITEM_WIDTH  10

static QFont gTime(gDefaultFont, FONT_SIZE);

TimelineEditor::TimelineEditor(QWidget *parent) :
        GraphWidget(parent),
        m_Position(0.0),
        m_Scale(0.01f),
        m_Step(MIN_STEP),
        m_OldPos(0),
        m_Drag(false),
        m_pClip(nullptr),
        m_CreateMenu(this),
        m_HoverTrack(0) {

    QRectF rect(-ITEM_WIDTH / 2, 2, ITEM_WIDTH, ITEM_WIDTH);
    m_Key.moveTo(rect.center().x(), rect.top());
    m_Key.lineTo(rect.right(), rect.center().y());
    m_Key.lineTo(rect.center().x(), rect.bottom());
    m_Key.lineTo(rect.left(), rect.center().y());
    m_Key.lineTo(rect.center().x(), rect.top());

    setFocusPolicy(Qt::StrongFocus);
    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(on_customContextMenuRequested(QPoint)));

    m_pAddKey       = m_CreateMenu.addAction(tr("Add Key"), this, SLOT(onAddKey()));
    m_pDeleteKeys   = m_CreateMenu.addAction(tr("Delete Key(s)"), this, SLOT(onDeleteKey()));
}

void TimelineEditor::draw(QPainter &painter, const QRect &r) {
    m_Head  = QRect(0, 0, r.width(), HEIGHT);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(117, 117, 117));
    painter.drawRoundedRect(r, 3, 3);

    painter.setPen(QColor(255, 255, 255));
    painter.setFont(gTime);

    painter.translate(-m_Translate.x(), -m_Translate.y());

    if(m_pClip) {
        int32_t y = HEADER;
        uint32_t i  = 0;
        for(auto &it : m_pClip->m_Tracks) {
            if(m_HoverTrack == i) {
                //painter.setPen(Qt::NoPen);
                //painter.setBrush(QColor(3, 155, 229));
                //painter.drawRect(0, y, width(), HEIGHT);
            }
            i++;

            painter.setPen(QColor(255, 255, 255));

            for(auto &key : it.curve) {
                bool found  = false;
                foreach(Select select, m_Selected) {
                    if(&key == select.key) {
                        found   = true;
                        break;
                    }
                }
                painter.setBrush((found) ? QColor(3, 155, 229) : QColor(96, 96, 96));
                int32_t x   = ((key.mPosition / 1000.0f) / m_Scale) * m_Step + 40;
                painter.translate(x, y);
                painter.drawPath(m_Key);
                painter.translate(-x,-y);
            }
            y += HEIGHT;

            painter.setPen(QColor(96, 96, 96));
            painter.translate(m_Translate.x(), 0);
            painter.drawLine(0, y, r.width(), y);
            painter.translate(-m_Translate.x(), 0);
        }
    }
    painter.resetTransform();

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(96, 96, 96));
    painter.drawRoundedRect(QRect(0, 0, width(), HEADER), 3, 3);

    painter.setPen(QColor(96, 96, 96));
    painter.drawLine(0, HEADER, r.width(), HEADER);

    painter.translate(-m_Translate.x(), 0);

    int32_t w   = MAX(width(), clipWidth());
    uint8_t counter = 4;
    for(int32_t i = 0; i < w; i += m_Step) {
        painter.setPen(QColor(255, 255, 255));
        int32_t l   = i + 40;
        if(counter == 4) {
            QRect rect(QPoint(l - 10, 0), QSize(40, FONT_SIZE + 4));
            QString number  = QString::number(static_cast<float>(i / m_Step) * m_Scale, 'f', 2).replace('.', ':');

            painter.drawText(rect, Qt::AlignLeft, number);

            painter.drawLine(l, 12, l, HEADER);
            painter.setPen(QColor(255, 255, 255, 64));
            painter.drawLine(l, HEADER, l, height());
            counter = 0;
        } else {
            painter.drawLine(l, 15, l, HEADER);
            painter.setPen(QColor(255, 255, 255, 32));
            painter.drawLine(l, HEADER, l, height());
            counter++;
        }
    }
    // Draw Position
    painter.setPen(QColor(255, 0, 0));
    int32_t x   = static_cast<int32_t>(m_Position / m_Scale) * m_Step + 40;
    painter.drawLine(x, 0, x, r.height());

    painter.resetTransform();
}

void TimelineEditor::setClip(AnimationClip *clip) {
    m_pClip = clip;

    update();

    emit scaled();
}

void TimelineEditor::setPosition(uint32_t ms) {
    m_Position  = static_cast<float>(ms) / 1000.0f;

    update();
}

int32_t TimelineEditor::clipWidth() {
    uint32_t width  = 0;
    if(m_pClip) {
        for(auto it : m_pClip->m_Tracks) {
            width   = MAX(((it.curve.back().mPosition / 1000.0f) / m_Scale) * m_Step + 80, width);
        }
    }
    return width;
}

int32_t TimelineEditor::clipHeight() {
    uint32_t height = HEADER;
    if(m_pClip) {
        height  += m_pClip->m_Tracks.size() * HEIGHT;
    }
    return height;
}

void TimelineEditor::setHovered(int32_t index) {
    m_HoverTrack    = index;
    update();
}

void TimelineEditor::onHScrolled(int value) {
    m_Translate.setX(value);
    update();
}

void TimelineEditor::onVScrolled(int value) {
    m_Translate.setY(value);
    update();
}

void TimelineEditor::onAddKey() {
    if(m_pClip) {
        int32_t y  = HEADER;
        for(auto &it : m_pClip->m_Tracks) {
            QRect r(0, y, width(), HEIGHT);
            if(r.contains(m_Point)) {
                KeyFrame key;

                float r = MAX(round((m_Point.x() - 40) / static_cast<float>(m_Step)), 0.0f);
                key.mPosition   = round(r * m_Scale * 1000.0f);

                VariantAnimation anim;
                anim.setKeyFrames(it.curve);
                anim.setCurrentTime(key.mPosition);

                key.mValue  = anim.currentValue();

                /// \todo build support points

                it.curve.push_back(key);
                it.curve.sort(AnimationClip::compare);

                emit changed();

                update();
            }
            y += HEIGHT;
        }
    }
}

void TimelineEditor::onDeleteKey() {
    if(m_pClip && checkKeys(m_Point)) {
        deleteSelected();
    }
}

void TimelineEditor::on_customContextMenuRequested(const QPoint &pos) {
    m_Point = pos;

    m_pAddKey->setEnabled(false);
    m_pDeleteKeys->setEnabled(false);
    if(m_pClip) {
        int32_t y   = HEADER;
        for(auto &it : m_pClip->m_Tracks) {
            QRect r(0, y, width(), HEIGHT);
            if(r.contains(m_Point)) {
                m_pAddKey->setEnabled(true);
            }
            for(auto &key : it.curve) {
                int32_t x   = ((key.mPosition / 1000.0f) / m_Scale) * m_Step + 40;
                r           = QRect (x - ITEM_WIDTH / 2, y, ITEM_WIDTH, HEIGHT);
                if(r.contains(pos)) {
                    m_pDeleteKeys->setEnabled(true);
                }
            }
            y += HEIGHT;
        }
    }

    m_CreateMenu.exec(mapToGlobal(pos));
}

void TimelineEditor::wheelEvent( QWheelEvent *pe ) {
    if(pe->delta() > 0) {
        m_Step += 1;
        if(m_Step > MAX_STEP) {
            if(m_Scale > 0.01f) {
                m_Step  = MIN_STEP;
                m_Scale/= 5;
            }
        }
    } else {
        m_Step -= 1;
        if(m_Step < MIN_STEP) {
            m_Step  = MAX_STEP;
            m_Scale*= 5;
        }
    }
    emit scaled();

    update();
}

void TimelineEditor::mouseMoveEvent( QMouseEvent *pe ) {
    if(m_pClip) {
        if(m_Drag) {
            if(m_Selected.isEmpty()) {
                m_Position  = MAX(round((pe->x() - 40) / static_cast<float>(m_Step)), 0) * m_Scale;
                emit moved(static_cast<uint32_t>(m_Position * 1000.0f) + 1);
            } else {
                float delta = round((pe->x() - m_OldPos) / m_Step) * m_Scale * 1000.0f;
                foreach(Select select, m_Selected) {
                    select.key->mPosition = select.pos + delta;
                }
                emit changed();
            }
        } else {
            m_HoverTrack   = -1;
            int32_t y   = HEADER;
            uint32_t i  = 0;
            for(auto &it : m_pClip->m_Tracks) {
                QRect r(0, y - m_Translate.y(), width(), HEIGHT);
                if(r.contains(pe->pos())) {
                    m_HoverTrack   = i;
                    emit hovered(m_HoverTrack);
                    break;
                }
                i++;
                y += HEIGHT;
            }
        }
        update();
    }
}

void TimelineEditor::mousePressEvent( QMouseEvent *pe ) {
    if(pe->button() == Qt::LeftButton) {
        m_Selected.clear();

        if(!checkKeys(pe->pos())) {
            if(m_Head.contains(pe->pos())) {
                m_Position  = MAX(round((pe->x() - 40) / static_cast<float>(m_Step)), 0) * m_Scale;
                m_Drag  = true;
                emit moved(static_cast<uint32_t>(m_Position * 1000.0f));
            }
        } else {
            m_Drag  = true;
            m_OldPos= pe->x();
        }
        update();
    }
}

void TimelineEditor::mouseReleaseEvent( QMouseEvent *pe ) {
    if(pe->button() == Qt::LeftButton) {
        m_Drag  = false;
    }
}

void TimelineEditor::mouseDoubleClickEvent( QMouseEvent *pe ) {
    if(pe->button() == Qt::LeftButton && m_pClip) {
        m_Point = pe->pos();

        onAddKey();
    }
}

void TimelineEditor::keyPressEvent( QKeyEvent *pe ) {
    if(pe->key() == Qt::Key_Delete && m_pClip) {
        deleteSelected();
    }
}

void TimelineEditor::resizeEvent( QResizeEvent *pe ) {
    emit scaled();
}

bool TimelineEditor::checkKeys(const QPoint &pos) {
    if(m_pClip) {
        int32_t y   = HEADER;
        for(auto &it : m_pClip->m_Tracks) {
            for(auto &key : it.curve) {
                int32_t x   = ((key.mPosition / 1000.0f) / m_Scale) * m_Step + 40;

                QRect r(x - ITEM_WIDTH / 2 - m_Translate.x(), y - m_Translate.y(), ITEM_WIDTH, HEIGHT);
                if(r.contains(pos)) {
                    Select select;
                    select.key  = &key;
                    select.pos  = key.mPosition;
                    m_Selected.push_back(select);
                    return true;
                }
            }
            y += HEIGHT;
        }
    }

    return false;
}

void TimelineEditor::deleteSelected() {
    for(auto &it : m_pClip->m_Tracks) {
        for(auto key = it.curve.begin(); key != it.curve.end(); key) {
            bool selected   = false;
            foreach(Select select, m_Selected) {
                if(select.key == &(*key)) {
                    selected    = true;
                }
            }
            if(selected) {
                key = it.curve.erase(key);
            } else {
                key++;
            }
        }
    }
    m_Selected.clear();
    update();
}
