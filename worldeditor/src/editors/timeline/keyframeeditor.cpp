#include "keyframeeditor.h"

#include <QGraphicsWidget>
#include <QGraphicsView>
#include <QGraphicsLinearLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QScrollBar>
#include <QSettings>

#include "animationclipmodel.h"
#include "timelinescene.h"
#include "ui/treerow.h"
#include "ui/ruler.h"
#include "ui/playhead.h"

KeyFrameEditor::KeyFrameEditor(QWidget *parent) :
    QWidget(parent),
    m_splitter(new QSplitter(this)),
    m_model(nullptr),
    m_scene(new TimelineScene(this)),
    m_treeHeader(new QGraphicsView(this)),
    m_treeView(new QGraphicsView(this)),
    m_timelineHeader(new QGraphicsView(this)),
    m_timelineView(new QGraphicsView(this)) {

    QColor color(117, 117, 117);

    readSettings();

    m_treeHeader->setFrameShape(QFrame::NoFrame);
    m_treeHeader->setFixedHeight(ROW);
    m_treeHeader->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_treeHeader->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_treeHeader->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_treeHeader->setScene(m_scene);
    m_treeHeader->setBackgroundBrush(color);
    m_treeHeader->setSceneRect(QRectF(0, 0, TREE_WIDTH, ROW));

    m_treeView->setFrameShape(QFrame::NoFrame);
    m_treeView->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_treeView->setScene(m_scene);
    m_treeView->setBackgroundBrush(color);
    m_treeView->setSceneRect(QRectF(0, ROW, TREE_WIDTH, m_treeView->height()));

    m_timelineHeader->setFrameShape(QFrame::NoFrame);
    m_timelineHeader->setFixedHeight(ROW);
    m_timelineHeader->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_timelineHeader->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_timelineHeader->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_timelineHeader->setScene(m_scene);
    m_timelineHeader->setBackgroundBrush(color);
    m_timelineHeader->setSceneRect(QRectF(TREE_WIDTH, 0, m_timelineHeader->width(), ROW));

    m_timelineView->setFrameShape(QFrame::NoFrame);
    m_timelineView->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_timelineView->setScene(m_scene);
    m_timelineView->setBackgroundBrush(color);
    m_timelineView->setSceneRect(QRectF(TREE_WIDTH, ROW, m_timelineView->width(), m_timelineView->height()));

    QVBoxLayout *treeLayout = new QVBoxLayout;
    treeLayout->setContentsMargins(QMargins(0, 0, 0, 0));
    treeLayout->setSpacing(0);
    treeLayout->addWidget(m_treeHeader);
    treeLayout->addWidget(m_treeView);

    QVBoxLayout *contentLayout = new QVBoxLayout;
    contentLayout->setContentsMargins(QMargins(0, 0, 0, 0));
    contentLayout->setSpacing(0);
    contentLayout->addWidget(m_timelineHeader);
    contentLayout->addWidget(m_timelineView);

    QWidget *treeWidget = new QWidget(m_splitter);
    treeWidget->setLayout(treeLayout);
    treeWidget->setMaximumWidth(TREE_WIDTH);

    QWidget *contentWidget = new QWidget(m_splitter);
    contentWidget->setLayout(contentLayout);

    m_splitter->addWidget(treeWidget);
    m_splitter->addWidget(contentWidget);

    QVBoxLayout *rootLayout = new QVBoxLayout;
    rootLayout->setContentsMargins(QMargins(0, 0, 0, 0));
    rootLayout->addWidget(m_splitter);

    setLayout(rootLayout);

    connect(m_scene->rootWidget(), &QGraphicsWidget::geometryChanged, this, [this]() {
        const QRectF rect = m_scene->rootWidget()->rect();

        m_treeView->setSceneRect(QRectF(0, ROW, TREE_WIDTH, rect.height()));

        m_timelineHeader->setSceneRect(QRectF(TREE_WIDTH, 0, rect.width() - TREE_WIDTH, ROW));

        m_timelineView->setSceneRect(QRectF(TREE_WIDTH, ROW, rect.width() - TREE_WIDTH, rect.height()));

        m_scene->playHead()->setHeight(rect.height());
    });

    connect(m_treeView->verticalScrollBar(), &QScrollBar::valueChanged, this,
            [this](int value) {
        m_timelineView->verticalScrollBar()->setValue(value);
    });

    connect(m_timelineView->verticalScrollBar(), &QScrollBar::valueChanged, this,
            [this](int value) {
        m_treeView->verticalScrollBar()->setValue(value);
    });

    connect(m_timelineView->horizontalScrollBar(), &QScrollBar::valueChanged, this,
            [this](int value) {
        m_timelineHeader->horizontalScrollBar()->setValue(value);

        int translateX = MAX(value - TREE_WIDTH - OFFSET, 0);
        m_scene->rulerWidget()->setTranslateX(translateX);
    });

    connect(m_scene, &TimelineScene::rowSelectionChanged, this,
            [this]() {
        QStringList list;
        for(auto &it : m_scene->selectedIndexes()) {
            if(m_model) {
                list << m_model->targetPath(it);
            }
        }
        emit rowsSelected(list);
    });

    connect(m_scene, &TimelineScene::headPositionChanged, this, &KeyFrameEditor::headPositionChanged);
    connect(m_scene, &TimelineScene::keySelectionChanged, this, &KeyFrameEditor::keySelectionChanged);

    connect(m_scene, &TimelineScene::keyPositionChanged, this, &KeyFrameEditor::onKeyPositionChanged);
    connect(m_scene, &TimelineScene::insertKeyframe, this, &KeyFrameEditor::onInsertKeyframe);
    connect(m_scene, &TimelineScene::deleteSelectedKey, this, &KeyFrameEditor::onDeleteSelectedKey);
    connect(m_scene, &TimelineScene::removeSelectedProperty, this, &KeyFrameEditor::onRemoveProperties);
}

KeyFrameEditor::~KeyFrameEditor() {
    writeSettings();
}

void KeyFrameEditor::readSettings() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    QVariant value = settings.value("timeline.splitter");
    if(value.isValid()) {
        m_splitter->restoreState(value.toByteArray());
    }
}

void KeyFrameEditor::writeSettings() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    settings.setValue("timeline.splitter", m_splitter->saveState());
}

void KeyFrameEditor::setModel(AnimationClipModel *model) {
    m_model = model;
    m_scene->setModel(m_model);
    connect(m_model, &AnimationClipModel::layoutChanged, this, &KeyFrameEditor::onClipUpdated, Qt::UniqueConnection);
}

void KeyFrameEditor::setPosition(uint32_t position) {
    m_scene->onPositionChanged(position);
}

void KeyFrameEditor::onClipUpdated() {
    QList<TreeRow *> items;
    for(int i = 0; i < m_scene->treeLayout()->count(); i++) {
        items.push_back(static_cast<TreeRow *>(m_scene->treeLayout()->itemAt(i)));
    }

    createTree(QModelIndex(), nullptr, items);
    m_scene->updateMaxDuration();

    // Delete unused
    for(auto it : items) {
        m_scene->treeLayout()->removeItem(it);
        m_scene->timelineLayout()->removeItem(it->timelineItem());
        delete it;
    }
}

void KeyFrameEditor::createTree(const QModelIndex &parentIndex, TreeRow *parent, QList<TreeRow *> &items) {
    if(m_model && m_model->clip()) {
        AnimationTrackList &tracks = m_model->clip()->m_Tracks;
        for(int i = 0 ; i < m_model->rowCount(parentIndex); i++) {
            QModelIndex index = m_model->index(i, 0, parentIndex);

            TreeRow *track = nullptr;
            for(auto it : items) {
                if(it->index() == index) {
                    track = it;
                    items.removeOne(track);
                    break;
                }
            }

            if(track == nullptr) {
                QVariant data = m_model->data(index, Qt::DisplayRole);
                track = new TreeRow(m_scene, parent);
                track->setName(data.toString());
                track->insertToLayout(m_scene->treeLayout()->count());

                if(parent) {
                    parent->addChild(track);
                }
            }

            auto it = tracks.begin();
            if(parentIndex.isValid()) {
                advance(it, parentIndex.row());
            } else {
                advance(it, index.row());
            }
            track->setTrack(&(*it), index);

            int count = m_model->rowCount(parentIndex);
            if(count > 0) {
                createTree(index, track, items);
            }
        }
    }
}

void KeyFrameEditor::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    Ruler *ruler = m_scene->rulerWidget();
    ruler->setMinimumHeight(event->size().height());
    ruler->setMinimumWidth(event->size().width());
}

void KeyFrameEditor::onRemoveProperties() {
    m_model->removeItems(m_scene->selectedIndexes());
}

void KeyFrameEditor::onKeyPositionChanged(float delta) {
    if(!m_model->isReadOnly()) {
        for(auto &it : m_scene->selectedKeyframes()) {
            it->setPosition(it->originPosition());
        }
        UndoManager::instance()->push(new UndoKeyPositionChanged(delta, m_scene, tr("Set Keyframe Time")));
    }
}

void KeyFrameEditor::onInsertKeyframe(int row, int col, float position) {
    if(!m_model->isReadOnly() && row >= 0) {
        if(!m_model->clip()->m_Tracks.empty()) {
            UndoManager::instance()->push(new UndoInsertKey(row, col, position, m_model, tr("Insert Keyframe")));
        }
    }
}

void KeyFrameEditor::onDeleteSelectedKey() {
    if(!m_model->isReadOnly()) {
        UndoManager::instance()->push(new UndoDeleteSelectedKey(m_scene, tr("Delete Selected Keyframe")));
    }
}

void UndoKeyPositionChanged::undo() {
    QSet<float> positions;
    QSet<TimelineRow *> rows;
    for(auto &it : m_scene->selectedKeyframes()) {
        float pos = it->position() - m_delta;
        it->setPosition(pos);
        positions.insert(pos);
        rows.insert(it->row()->timelineItem());
    }

    for(auto &row : rows) {
        AnimationTrack *track = row->track();

        track->fixCurves();
        row->updateKeys();
        for(auto position : positions) {
            KeyFrame *key = row->keyAtPosition(position, false);
            if(key) {
                key->setSelected(true);
            }
        }
    }

    m_scene->updateMaxDuration();
    m_scene->update();
    emit m_scene->model()->changed();
}

void UndoKeyPositionChanged::redo() {
    QSet<float> positions;
    QSet<TimelineRow *> rows;
    for(auto &it : m_scene->selectedKeyframes()) {
        float pos = it->position() + m_delta;
        it->setPosition(pos);
        positions.insert(pos);
        rows.insert(it->row()->timelineItem());
    }

    for(auto &row : rows) {
        AnimationTrack *track = row->track();

        track->fixCurves();
        row->updateKeys();
        for(auto position : positions) {
            KeyFrame *key = row->keyAtPosition(position, false);
            if(key) {
                key->setSelected(true);
            }
        }
    }

    m_scene->updateMaxDuration();
    m_scene->update();
    emit m_scene->model()->changed();
}

void UndoDeleteSelectedKey::undo() {
    QSet<float> positions;
    QSet<TimelineRow *> rows;
    for(auto &it : m_keys) {
        TreeRow *tree =  m_scene->row(it.row, it.column);
        if(tree) {
            TimelineRow *row = tree->timelineItem();

            rows.insert(row);

            AnimationTrack *track = row->track();
            auto &curves = track->curves();
            auto &curve = curves[it.column];
            curve.m_Keys.insert(curve.m_Keys.begin() + it.index, it.key);

            positions.insert(it.key.m_Position);

            auto &keys = row->keys();
            keys[it.index].setSelected(true);

            if(tree->parentRow()) {
                tree->parentRow()->update();
            }
        }
    }

    for(auto &row : rows) {
        AnimationTrack *track = row->track();

        track->fixCurves();
        row->updateKeys();
        for(auto position : positions) {
            KeyFrame *key = row->keyAtPosition(position, false);
            if(key) {
                key->setSelected(true);
            }
        }
    }

    m_scene->updateMaxDuration();
    m_scene->update();
    emit m_scene->model()->changed();
}

void UndoDeleteSelectedKey::redo() {
    m_keys.clear();
    for(auto &it : m_scene->selectedKeyframes()) {
        TimelineRow *row = it->row()->timelineItem();
        AnimationTrack *track = row->track();
        if(track) {
            AnimationCurve::KeyFrame *k = it->key();
            if(k) {
                QModelIndex index = it->row()->index();
                int column = index.row();
                auto &curves = track->curves();
                auto &curve = curves[column];

                auto key = std::find(curve.m_Keys.begin(), curve.m_Keys.end(), *k);
                if(key != curve.m_Keys.end()) {
                    FrameData data;
                    data.key = *key;
                    data.index = row->keys().indexOf(*it);
                    data.column = column;
                    data.row = index.parent().row();
                    m_keys.push_back(data);

                    curve.m_Keys.erase(key);
                }
            }

            track->fixCurves();
            row->updateKeys();
        }
    }

    m_scene->updateMaxDuration();
    m_scene->update();
    emit m_scene->model()->changed();
}
