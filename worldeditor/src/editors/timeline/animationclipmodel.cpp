#include "animationclipmodel.h"

#include <resources/animationclip.h>
#include <resources/animationstatemachine.h>
#include <components/animationcontroller.h>
#include <components/actor.h>

#include "assetmanager.h"

#include <iterator>

#include <QColor>

const QStringList components = {"x", "y", "z", "w"};

AnimationClipModel::AnimationClipModel(QObject *parent) :
        QAbstractItemModel(parent),
        m_pController(nullptr),
        m_pStateMachine(nullptr),
        m_pClip(nullptr),
        m_Position(0.0f) {

}

void AnimationClipModel::setController(AnimationController *controller) {
    m_pController = controller;
    m_pStateMachine = nullptr;
    m_pClip = nullptr;
    m_Clips.clear();

    if(m_pController) {
        m_pStateMachine = m_pController->stateMachine();
        if(m_pStateMachine) {

            for(auto it : m_pStateMachine->states()) {
                QFileInfo info(AssetManager::instance()->guidToPath(Engine::reference(it->m_clip)).c_str());
                m_Clips[info.baseName()] = it->m_clip;
            }
            if(!m_Clips.isEmpty()) {
                setClip(m_Clips.begin().key());
                return;
            }
        }
    }
    emit layoutAboutToBeChanged();
    emit layoutChanged();
}

void AnimationClipModel::setClip(const QString &clip) {
    m_pClip = m_Clips.value(clip);
    if(m_pClip) {
        m_pController->setClip(m_pClip);

        emit layoutAboutToBeChanged();
        emit layoutChanged();

        setPosition(0.0f);
    }
}

QVariant AnimationClipModel::data(const QModelIndex &index, int role) const {
    if(!index.isValid() || m_pClip == nullptr) {
        return QVariant();
    }

    switch(role) {
        case Qt::EditRole:
        case Qt::ToolTipRole:
        case Qt::DisplayRole: {
            auto it = m_pClip->m_Tracks.begin();
            if(index.internalPointer() == &m_pClip->m_Tracks) {
                advance(it, index.row());
                if(it != m_pClip->m_Tracks.end()) {
                    QStringList lst = QString::fromStdString(it->path()).split('/');
                    lst.pop_back();
                    QString actor = lst.last();

                    return QString("%1 : %2").arg(actor, QString(it->property().c_str()).replace('_', ""));
                }
            } else {
                advance(it, index.parent().row());
                int component = std::next(it->curves().begin(), index.row())->first;
                if(component >= 0) {
                    return QString("%1.%2").arg(it->property().c_str(), components.at(component));
                }
            }
        } break;
        default: break;
    }

    return QVariant();
}

QVariant AnimationClipModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0: return "";
        }
    }
    return QVariant();
}

int AnimationClipModel::columnCount(const QModelIndex &) const {
    return 1;
}

QModelIndex AnimationClipModel::index(int row, int column, const QModelIndex &parent) const {
    if(m_pClip) {
        AnimationTrackList *list = &m_pClip->m_Tracks;
        if(!parent.isValid()) {
            return createIndex(row, column, list);
        } else {
            if(parent.internalPointer() == list) {
                if(static_cast<uint32_t>(parent.row()) < list->size()) {
                    void *ptr = &(std::next(list->begin(), parent.row())->curves());
                    return createIndex(row, column, ptr);
                }
            }
        }
    }
    return QModelIndex();
}

QModelIndex AnimationClipModel::parent(const QModelIndex &index) const {
    if(index.isValid() && m_pClip) {
        AnimationTrackList *list  = &m_pClip->m_Tracks;
        if(index.internalPointer() != list) {
            int row = 0;
            for(auto &it : m_pClip->m_Tracks) {
                if(index.internalPointer() == &(it.curves())) {
                    break;
                }
                row++;
            }
            return createIndex(row, 0, list);
        }

    }
    return QModelIndex();
}

int AnimationClipModel::rowCount(const QModelIndex &parent) const {
    if(m_pClip) {
        AnimationTrackList *list = &m_pClip->m_Tracks;
        if(!list->empty()) {
            if(!parent.isValid()) {
                return static_cast<int32_t>(list->size());
            } else {
                if(parent.internalPointer() == list) {
                    if(static_cast<uint32_t>(parent.row()) < list->size()) {
                        int result = static_cast<int32_t>(std::next(list->begin(), parent.row())->curves().size());
                        return (result > 1) ? result : 0;
                    }
                }
            }
        }
    }
    return 0;
}

float AnimationClipModel::position() const {
    return m_Position;
}

void AnimationClipModel::setPosition(float value) {
    m_Position = value;

    if(m_pController) {
        m_pController->setPosition(m_Position);
    }

    emit positionChanged(m_Position);
}

void AnimationClipModel::removeItems(const QModelIndexList &list) {
    if(!isReadOnly()) {
        QList<int> rows;
        foreach(const QModelIndex &index, list) {
            if(!index.parent().isValid()) { // Not sub component
                rows.push_back(index.row());
            }
        }
        UndoManager::instance()->push(new UndoRemoveItems(rows, this, tr("Remove Properties")));
    }
}

AnimationCurve::KeyFrame *AnimationClipModel::key(int32_t track, int32_t col, int32_t index) {
    if(track >= 0) {
        if(m_pClip->m_Tracks.size() > size_t(track)) {
            AnimationTrack &t = *std::next(m_pClip->m_Tracks.begin(), track);
            if(t.curves().size() > col && t.curves()[col].m_Keys.size() > index) {
                return &t.curves()[col].m_Keys[index];
            }
        }
    }
    return nullptr;
}

AnimationTrack &AnimationClipModel::track(int32_t track) {
    return *std::next(m_pClip->m_Tracks.begin(), track);
}

void AnimationClipModel::commitKey(int row, int col, int index, float value, float left, float right, int position) {
    AnimationCurve::KeyFrame *k = key(row, col, index);
    if(!isReadOnly() && k) {
        UndoManager::instance()->push(new UndoUpdateKey(row, col, index, value, left, right, position, this, tr("Update Key")));
    }
}

bool AnimationClipModel::isReadOnly() const {
    return false;
}

void AnimationClipModel::updateController() {
    m_pController->setStateMachine(m_pStateMachine);
    m_pController->setPosition(m_Position);
    m_pController->setClip(m_pClip);

    emit changed();

    emit layoutAboutToBeChanged();
    emit layoutChanged();
}

void UndoUpdateKey::undo() {
    AnimationCurve::KeyFrame *k = m_pModel->key(m_Row, m_Column, m_Index);
    if(k) {
        *k = m_Key;

        m_pModel->updateController();
    }
}

void UndoUpdateKey::redo() {
    AnimationTrack &track = (*std::next(m_pModel->clip()->m_Tracks.begin(), m_Row));

    AnimationCurve::KeyFrame *k = &track.curves()[m_Column].m_Keys[m_Index];
    if(k) {
        m_Key = *k;

        k->m_Value = m_Value;
        k->m_LeftTangent = m_Left;
        k->m_RightTangent = m_Right;
        k->m_Position = (float)m_Position / (float)track.duration();

        m_pModel->updateController();
    }
}

void UndoRemoveItems::undo() {
    int i = 0;
    for(auto track : m_Tracks) {
        auto it = std::next(m_pModel->clip()->m_Tracks.begin(), m_Rows.at(i));
        m_pModel->clip()->m_Tracks.insert(it, track);
        i++;
    }
    m_pModel->updateController();
}

void UndoRemoveItems::redo() {
    m_Tracks.clear();
    for(int row : m_Rows) {
        auto it = m_pModel->clip()->m_Tracks.begin();
        advance(it, row);

        m_Tracks.push_back(*it);

        m_pModel->clip()->m_Tracks.erase(it);
    }
    m_pModel->updateController();
}

void UndoUpdateItems::undo() {
    redo();
}

void UndoUpdateItems::redo() {
    AnimationClip *clip = m_pModel->clip();
    if(clip) {
        AnimationTrackList save = clip->m_Tracks;
        clip->m_Tracks = m_Tracks;
        m_Tracks = save;
        m_pModel->updateController();
    }
}
