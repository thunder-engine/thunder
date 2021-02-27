#include "keyframeeditor.h"

#include <QGraphicsWidget>
#include <QGraphicsView>
#include <QGraphicsLinearLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QScrollBar>
#include <QSettings>

#include <QDebug>

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

    connect(m_scene, &TimelineScene::headPositionChanged, this,
            [this](float value) {
        if(m_model) {
            m_model->blockSignals(true);
            m_model->setPosition(value);
            m_model->blockSignals(false);
        }
    });

    connect(m_scene, &TimelineScene::keySelectionChanged, this, &KeyFrameEditor::keySelectionChanged);
    connect(m_scene, &TimelineScene::keyPositionChanged, this, &KeyFrameEditor::onKeyPositionChanged);
    connect(m_scene, &TimelineScene::insertKeyframe, this, &KeyFrameEditor::onInsertKeyframe);
    connect(m_scene, &TimelineScene::deleteSelectedKey, this, &KeyFrameEditor::onDeleteSelectedKey);
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
    connect(m_model, &AnimationClipModel::layoutChanged, this, &KeyFrameEditor::onClipUpdated);
    connect(m_model, &AnimationClipModel::positionChanged, m_scene, &TimelineScene::onPositionChanged);
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
        m_scene->timelineLayout()->removeItem(&it->timelineItem());
        delete it;
    }

    m_scene->setReadOnly(m_model->isReadOnly());
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
    if(!m_scene->isReadOnly()) {
        m_model->removeItems(m_scene->selectedIndexes());
    }
}

void KeyFrameEditor::onKeyPositionChanged(float delta) {
    if(!m_scene->isReadOnly()) {
        for(auto &it : m_scene->selectedKeyframes()) {
            it->setPosition(it->originPosition());
        }
        UndoManager::instance()->push(new UndoKeyPositionChanged(delta, m_scene, tr("Set Keyframe Time")));
    }
}

void KeyFrameEditor::onInsertKeyframe(int row, int col, float position) {
    if(!m_scene->isReadOnly() && row >= 0) {
        if(!m_model->clip()->m_Tracks.empty()) {
            UndoManager::instance()->push(new UndoInsertKey(row, col, position, m_model, tr("Insert Key")));
        }
    }
}

void KeyFrameEditor::onDeleteSelectedKey() {
    if(!m_scene->isReadOnly()) {
        UndoManager::instance()->push(new UndoDeleteSelectedKey(m_scene, tr("Delete Selected Keyframe")));
    }
}

void UndoInsertKey::undo() {
    auto &curves = (*std::next(m_model->clip()->m_Tracks.begin(), m_row)).curves();

    int i = (m_column == -1) ? 0 : m_column;
    for(auto index : m_indices) {
        auto &curve = curves[i];
        auto it = std::next(curve.m_Keys.begin(), index);

        curve.m_Keys.erase(it);
        i++;
    }

    m_model->updateController();
}

void UndoInsertKey::redo() {
    m_indices.clear();

    AnimationTrack &track = (*std::next(m_model->clip()->m_Tracks.begin(), m_row));
    auto &curves = track.curves();

    if(m_column > -1) {
        auto &curve = curves[m_column];
        insertKey(curve);
    } else {
        for(uint32_t i = 0; i < curves.size(); i++) {
            auto &curve = curves[i];
            insertKey(curve);
        }
    }

    m_model->updateController();
}

void UndoInsertKey::insertKey(AnimationCurve &curve) {
    AnimationCurve::KeyFrame key;
    key.m_Position = m_position;
    key.m_Value = curve.value(key.m_Position);
    key.m_LeftTangent = key.m_Value;
    key.m_RightTangent = key.m_Value;

    int index = 0;
    for(auto it : curve.m_Keys) {
        if(it.m_Position > key.m_Position) {
            break;
        }
        index++;
    }
    m_indices.push_back(index);
    curve.m_Keys.insert(curve.m_Keys.begin() + index, key);
}

void UndoKeyPositionChanged::undo() {
    for(auto &it : m_scene->selectedKeyframes()) {
        it->setPosition(it->position() - m_delta);
    }

    for(auto &it : m_scene->selectedKeyframes()) {
        it->row()->timelineItem().fixCurve();
    }
    m_scene->updateMaxDuration();
    m_scene->update();
}

void UndoKeyPositionChanged::redo() {
    for(auto &it : m_scene->selectedKeyframes()) {
        it->setPosition(it->position() + m_delta);
    }

    for(auto &it : m_scene->selectedKeyframes()) {
        it->row()->timelineItem().fixCurve();
    }
    m_scene->updateMaxDuration();
    m_scene->update();
}

void UndoDeleteSelectedKey::undo() {
    for(auto &it : m_keys) {
        TreeRow *tree =  m_scene->row(it.row, it.column);
        if(tree) {
            TimelineRow &row = tree->timelineItem();

            AnimationTrack *track = row.track();
            auto &curves = track->curves();
            auto &curve = curves[it.column];
            curve.m_Keys.insert(curve.m_Keys.begin() + it.index, it.key);

            row.updateKeys();
            auto &keys = row.keys();
            keys[it.index].setSelected(true);
            m_scene->selectedKeyframes().push_back(&keys[it.index]);

            if(tree->parentRow()) {
                tree->parentRow()->update();
            }
            row.fixCurve();
        }
    }
    m_scene->updateMaxDuration();
    m_scene->update();
}

void UndoDeleteSelectedKey::redo() {
    m_keys.clear();
    auto &list = m_scene->selectedKeyframes();
    for(auto &it : list) {
        TimelineRow &row = it->row()->timelineItem();
        AnimationTrack *track = row.track();
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
                    data.index = row.keys().indexOf(*it);
                    data.column = column;
                    data.row = index.parent().row();
                    m_keys.push_back(data);

                    curve.m_Keys.erase(key);
                }
            }

            row.fixCurve();
        }
        it->setSelected(false);
    }

    list.clear();
    m_scene->updateMaxDuration();
    m_scene->update();
}
