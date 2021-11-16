#include "timelinescene.h"

#include <QGraphicsLinearLayout>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsWidget>
#include <QGraphicsView>
#include <QMenu>

#include <QDebug>

#include <animationclip.h>

#include "ui/treerow.h"
#include "ui/ruler.h"
#include "ui/playhead.h"

#include "animationclipmodel.h"

#define DRAG_SENSITIVITY 2

static QPointF invalidPos(-MAX_VALUE, -MAX_VALUE);

TimelineScene::TimelineScene(QWidget *editor) :
        QGraphicsScene(editor),
        m_layoutRoot(new QGraphicsLinearLayout(Qt::Horizontal)),
        m_layoutTree(new QGraphicsLinearLayout(Qt::Vertical)),
        m_layoutTimeline(new QGraphicsLinearLayout(Qt::Vertical)),
        m_model(nullptr),
        m_widgetRoot(new QGraphicsWidget),
        m_rulerItem(new Ruler),
        m_playHead(new PlayHead(m_rulerItem)),
        m_pressedKeyframe(nullptr),
        m_pressPos(invalidPos),
        m_pressKeyPosition(-MAX_VALUE),
        m_drag(false) {

    addItem(m_rulerItem);
    addItem(m_widgetRoot);
    addItem(m_playHead);

    m_layoutRoot->setSpacing(0);
    m_layoutRoot->setContentsMargins(0, 0, 0, 0);
    m_layoutRoot->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    m_layoutTree->setSpacing(0);
    m_layoutTree->setContentsMargins(0, 0, 0, 0);
    m_layoutTree->setMinimumWidth(TREE_WIDTH + OFFSET);
    m_layoutTree->setMaximumWidth(TREE_WIDTH + OFFSET);
    m_layoutTree->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_layoutTimeline->setSpacing(0);
    m_layoutTimeline->setContentsMargins(0, 0, 0, 0);
    m_layoutTimeline->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

    m_layoutRoot->addItem(m_layoutTree);
    m_layoutRoot->addItem(m_layoutTimeline);

    m_widgetRoot->setLayout(m_layoutRoot);

    m_rulerItem->setPos(TREE_WIDTH + OFFSET, 0);
    m_widgetRoot->setPos(0, ROW);
    m_playHead->setTime(0);

    connect(m_rulerItem, &Ruler::maxDurationChanged, this, &TimelineScene::onContentWidthChanged);
    connect(m_rulerItem, &Ruler::zoomChanged, this, &TimelineScene::onContentWidthChanged);
}

QGraphicsLinearLayout *TimelineScene::treeLayout() const {
    return m_layoutTree;
}

QGraphicsLinearLayout *TimelineScene::timelineLayout() const {
    return m_layoutTimeline;
}

QGraphicsWidget *TimelineScene::rootWidget() const {
    return m_widgetRoot;
}

Ruler *TimelineScene::rulerWidget() const {
    return m_rulerItem;
}

PlayHead *TimelineScene::playHead() const {
    return m_playHead;
}

QModelIndexList TimelineScene::selectedIndexes() const {
    return m_selectedRows;
}

QList<KeyFrame *> &TimelineScene::selectedKeyframes() {
    return m_selectedKeyframes;
}

void TimelineScene::clearSelection() {
    m_selectedKeyframes.clear();

    for(int i = 0; i < m_layoutTimeline->count(); i++) {
        TimelineRow *row = static_cast<TimelineRow *>(m_layoutTimeline->itemAt(i));
        for(auto &it : row->keys()) {
            it.setSelected(false);
        }
        row->update();
    }
}

TreeRow *TimelineScene::row(int row, int component) {
    int index = 0;
    for(int i = 0; i < m_layoutTree->count(); i++) {
        TreeRow *r = static_cast<TreeRow *>(m_layoutTree->itemAt(i));
        if(r->parentRow() == nullptr) {
            if(index == row) {
                return r->children().at(component);
            }
            index++;
        }
    }
    return nullptr;
}

void TimelineScene::updateMaxDuration() {
    int32_t duration = 0;
    for(int i = 0; i < m_layoutTimeline->count(); i++) {
        TimelineRow *tree = static_cast<TimelineRow *>(m_layoutTimeline->itemAt(i));
        if(tree->track()) {
            duration = MAX(duration, tree->track()->duration());
        }
    }
    m_rulerItem->setMaxDuration(duration);
}

void TimelineScene::setModel(AnimationClipModel *model) {
    m_model = model;
}

void TimelineScene::onPositionChanged(uint32_t time) {
    m_playHead->setTime((float)time / 1000.0f);
    update();
}

void TimelineScene::onContentWidthChanged() {
    uint32_t w = OFFSET * 2 + m_rulerItem->timeToScreen(m_rulerItem->maxDuration() / 1000.0f);

    m_layoutTimeline->setMinimumWidth(w);
    m_layoutTimeline->setMaximumWidth(w);

    m_rulerItem->setMinimumWidth(w);
    m_rulerItem->setMaximumWidth(w);
}

void TimelineScene::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if(event->button() == Qt::LeftButton) {
        m_pressPos = event->scenePos();

        for(int i = 0; i < m_layoutTree->count(); i++) {
            TreeRow *row = static_cast<TreeRow *>(m_layoutTree->itemAt(i));
            row->setSelected(false);
        }
        m_selectedRows.clear();

        const QList<QGraphicsItem *> hoverItems = items(m_pressPos);
        if(!hoverItems.isEmpty()) {
            QGraphicsItem *item = hoverItems.at(0);
            switch(item->type()) {
                case QGraphicsTextItem::Type:
                case RowItem::TreeItem: {
                    TreeRow *row = static_cast<TreeRow *>(item);
                    if(item->type() == QGraphicsTextItem::Type) {
                        row = static_cast<TreeRow *>(item->parentItem());
                    }
                    row->onRowPressed(event->scenePos());
                    row->setSelected(true);
                    m_selectedRows.push_back(row->index());
                    emit rowSelectionChanged();
                } break;
                case RowItem::TimelineItem: {
                    TimelineRow *r = static_cast<TimelineRow *>(item);

                    auto list = r->onRowPressed(event->scenePos());
                    if(!list.isEmpty()) {
                        m_pressedKeyframe = list.at(0);
                        if(m_pressedKeyframe) {
                            if(!m_pressedKeyframe->isSelected()) {
                                if((event->modifiers() & Qt::ControlModifier) == false) {
                                    clearSelection();
                                }

                                for(auto &it : list) {
                                    it->setSelected(true);
                                    m_selectedKeyframes.push_back(it);
                                }

                                int idx = r->keys().indexOf(*m_pressedKeyframe);
                                int row = -1;
                                int col = -1;

                                QModelIndex index = r->treeRow()->index();
                                if(index.parent().isValid()) {
                                    row = index.parent().row();
                                    col = index.row();
                                } else {
                                    row = index.row();
                                }

                                emit keySelectionChanged(row, col, idx);
                            }
                        }
                    } else {
                        clearSelection();
                    }
                } break;
                case RowItem::RulerItem: {
                    float x = event->scenePos().x() - m_rulerItem->x();
                    x = m_rulerItem->screenToTime(x, true);

                    m_playHead->setTime(x);
                    emit headPositionChanged(x * 1000.0f);
                    update();
                } break;
                default: break;
            }
        }
    }
}

void TimelineScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsScene::mouseReleaseEvent(event);
    if(event->button() == Qt::LeftButton) {
        if(m_drag) {
            m_drag = false;

            // clean root keys
            for(int i = 0; i < m_layoutTree->count(); i++) {
                TreeRow *row = static_cast<TreeRow *>(m_layoutTree->itemAt(i));
                if(row->parentRow() == nullptr) {
                    TimelineRow &item = row->timelineItem();
                    QList<KeyFrame *> outdate;
                    for(auto &key : item.keys()) {
                        if(key.isSelected()) {
                            for(auto &it : item.keys()) {
                                if(key.position() == it.position() && !it.isSelected()) {
                                    outdate.push_back(&it);
                                }
                            }
                        }
                    }
                    if(!outdate.isEmpty()) {
                        for(auto it : outdate) {
                            item.keys().removeOne(*it);
                        }
                        item.update();
                    }
                }
            }

            if(m_pressedKeyframe) {
                emit keyPositionChanged(m_pressedKeyframe->position() - m_pressKeyPosition);
            }
        }
        m_pressedKeyframe = nullptr;
        m_pressPos = invalidPos;
        m_pressKeyPosition = -MAX_VALUE;
    }
}

void TimelineScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsScene::mouseMoveEvent(event);
    for(int i = 0; i < m_layoutTree->count(); i++) {
        TreeRow *row = static_cast<TreeRow *>(m_layoutTree->itemAt(i));
        row->mouseHover(false);
    }

    for(auto &it : items(event->scenePos())) {
        if(it->type() == RowItem::TreeItem) {
            TreeRow *row = static_cast<TreeRow *>(it);
            row->mouseHover(true);
            break;
        } else if(it->type() == RowItem::TimelineItem) {
            TimelineRow *row = static_cast<TimelineRow *>(it);
            row->treeRow()->mouseHover(true);
            break;
        }
    }

    if(!m_drag && m_pressPos != invalidPos && (event->scenePos() - m_pressPos).manhattanLength() > DRAG_SENSITIVITY) {
        m_drag = true;

        if(m_pressedKeyframe && !m_model->isReadOnly()) {
            m_pressKeyPosition = m_pressedKeyframe->position();

            TreeRow *parent = m_pressedKeyframe->row()->parentRow();
            if(parent) {
                int count = 0;
                for(auto &it : parent->children()) {
                    KeyFrame *k = it->timelineItem().keyAtPosition(m_pressKeyPosition, false);
                    if(k) {
                        count++;
                    }
                }
            }

            for(auto &it : m_selectedKeyframes) {
                it->setOriginPosition(it->position());
            }
        }
    }

    if(m_drag) {
        float x = event->scenePos().x() - m_rulerItem->x();
        if(m_pressedKeyframe) {
            if(!m_model->isReadOnly()) {
                x = m_rulerItem->screenToTime(x, !(event->modifiers() & Qt::AltModifier));

                for(auto &it : m_selectedKeyframes) {
                    TimelineRow &row = it->row()->timelineItem();
                    AnimationTrack *track = row.track();

                    float delta = (x * 1000.0f / track->duration()) - m_pressKeyPosition;
                    it->setPosition(it->originPosition() + delta);
                }
            }
        } else {
            x = MAX(0.0f, m_rulerItem->screenToTime(x, true));
            m_playHead->setTime(x);
            emit headPositionChanged(x * 1000.0f);
            update();
        }
    }
}

void TimelineScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsScene::mouseDoubleClickEvent(event);
    if(event->button() == Qt::LeftButton) {
        const QList<QGraphicsItem *> hoverItems = items(event->scenePos());
        if(!hoverItems.isEmpty()) {
            QGraphicsItem *item = hoverItems.at(0);
            if(item->type() == RowItem::TimelineItem) {
                TimelineRow *r = static_cast<TimelineRow *>(item);
                TreeRow *tree = r->treeRow();
                if(tree) {
                    QModelIndex index = tree->index();

                    float pos = r->onRowDoubleClicked(event->scenePos());

                    int row = -1;
                    int col = -1;
                    if(index.parent().isValid()) {
                        row = index.parent().row();
                        col = index.row();
                    } else {
                        row = index.row();
                    }

                    emit insertKeyframe(row, col, pos);
                }
            }
        }
    }
}

void TimelineScene::wheelEvent(QGraphicsSceneWheelEvent *event) {
    QGraphicsScene::wheelEvent(event);
    if(event->delta() > 0) {
        m_rulerItem->zoomIn();
    } else {
        m_rulerItem->zoomOut();
    }
    m_playHead->updatePosition();
    update();
}

void TimelineScene::keyPressEvent(QKeyEvent *event) {
    QGraphicsScene::keyPressEvent(event);

    if(event->key() == Qt::Key_Delete) {
        emit deleteSelectedKey();
    }
}

void TimelineScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
    const QList<QGraphicsItem *> hoverItems = items(event->scenePos());
    if(!hoverItems.isEmpty()) {
        for(int i = 0; i < m_layoutTree->count(); i++) {
            TreeRow *row = static_cast<TreeRow *>(m_layoutTree->itemAt(i));
            row->setSelected(false);
        }
        m_selectedRows.clear();

        QMenu menu;

        QGraphicsItem *item = hoverItems.at(0);
        switch(item->type()) {
            case QGraphicsTextItem::Type:
            case RowItem::TreeItem: {
                TreeRow *row = static_cast<TreeRow *>(item);
                if(item->type() == QGraphicsTextItem::Type) {
                    row = static_cast<TreeRow *>(item->parentItem());
                }
                row->setSelected(true);
                m_selectedRows.push_back(row->index());

                menu.addAction(tr("Remove Property"), this, SIGNAL(removeSelectedProperty()));
            } break;
            default: break;
        }

        if(!menu.actions().isEmpty()) {
            menu.exec(event->screenPos());
        }
    }
}
