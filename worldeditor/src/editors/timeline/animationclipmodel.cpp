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
        m_isHighlighted(false),
        m_Position(0.0f),
        m_Row(0),
        m_Col(-1) {

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
                QFileInfo info(AssetManager::instance()->guidToPath(Engine::reference(it->m_pClip)).c_str());
                m_Clips[info.baseName()] = it->m_pClip;
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

        setRow((m_pClip->m_Tracks.size() > 0) ? 0 : -1);
    }
}

void AnimationClipModel::onExpanded(const QModelIndex &index) {
    m_Expands.append(index.row());

    emit layoutAboutToBeChanged();
    emit layoutChanged();
}

void AnimationClipModel::onCollapsed(const QModelIndex &index) {
    m_Expands.removeAll(index.row());

    emit layoutAboutToBeChanged();
    emit layoutChanged();
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
                    QString component = lst.last();
                    lst.pop_back();
                    QString actor = lst.last();

                    int32_t size = lst.size();

                    if(component.isEmpty()) {
                        component = QString::fromStdString(m_pController->actor()->name());
                        size = 0;
                    }

                    QString spaces;
                    for(int32_t i = 0; i < size; i++) {
                        spaces  += "    ";
                    }
                    return QString("%1%2 : %3").arg(spaces).arg(actor).arg(QString(it->property().c_str()).replace('_', ""));
                }
            } else {
                advance(it, index.parent().row());
                int component = std::next(it->curves().begin(), index.row())->first;
                if(component >= 0) {
                    return QString("%1.%2").arg(it->property().c_str()).arg(components.at(component));
                }
            }
        } break;
        case Qt::BackgroundColorRole: {
            if(m_isHighlighted && (index == m_HoverIndex)) {
                return QColor(229, 0, 0);
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

bool AnimationClipModel::isExpanded(int track) const {
    return (m_Expands.indexOf(track) > -1);
}

int AnimationClipModel::expandHeight(int track) const {
    int result = 0;
    if(m_pClip && track >= 0) {
        int i = 0;
        for(auto it : m_pClip->m_Tracks) {
            if(i >= track) {
                break;
            }
            result += 1;
            if(m_Expands.indexOf(i) > -1 && it.curves().size() > 1) {
                result += it.curves().size();
            }
            i++;
        }
    }
    return result;
}

QVariant AnimationClipModel::trackData(int track) const {
    if(m_pClip && track >= 0) {
        if(size_t(track) < m_pClip->m_Tracks.size()) {
            auto &curves = (*std::next(m_pClip->m_Tracks.begin(), track)).curves();

            QVariantList t;
            for(auto &it : curves) {
                QVariantList curve;
                curve.push_back(it.first);
                for(auto &k : it.second.m_Keys) {
                    curve.push_back(QVariantList({ k.m_Position, k.m_Type, k.m_Value, k.m_LeftTangent, k.m_RightTangent }));
                }
                t.push_back(curve);
            }
            return t;
        }
    }
    return QVariant();
}

void AnimationClipModel::setTrackData(int track, const QVariant &data) {
    if(m_pClip && track >= 0 && data.isValid()) {
        if(track < m_pClip->m_Tracks.size()) {
            UndoManager::instance()->push(new UndoUpdateKeys(track, data, this, "Update Keys"));
        }
    }
}

int AnimationClipModel::maxPosition(int track) {
    if(m_pClip && track >= 0) {
        if(!m_pClip->m_Tracks.empty()) {
            auto &curves = (*std::next(m_pClip->m_Tracks.begin(), track)).curves();

            int32_t result = 0;
            for(auto &it : curves) {
                if(!it.second.m_Keys.empty())
                result = MAX(static_cast<int32_t>(it.second.m_Keys.back().m_Position), result);
            }
            return result;
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
        m_pController->setPosition(1000 * m_Position);
    }

    emit positionChanged();
}

int AnimationClipModel::row() const {
    return m_Row;
}

void AnimationClipModel::setRow(int value) {
    m_Row = value;
    emit rowChanged();
}

int AnimationClipModel::col() const {
    return m_Col;
}

void AnimationClipModel::setCol(int value) {
    m_Col = value;
}

void AnimationClipModel::selectItem(const QModelIndex &index) {
    if(index.parent().isValid()) { // Sub component
        QModelIndex p = parent(index);
        if(p.isValid()) {
            setCol(index.row());
            setRow(p.row());
        }
    } else {
        setCol(-1);
        setRow(index.row());
    }
}

void AnimationClipModel::removeItems(const QModelIndexList &list) {
    QList<int> rows;
    foreach(const QModelIndex &index, list) {
        if(!index.parent().isValid()) { // Not sub component
            rows.push_back(index.row());
        }
    }
    UndoManager::instance()->push(new UndoRemoveItems(rows, this, "Remove Properties"));
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

void AnimationClipModel::onUpdateKey(int row, int col, int index, float value, float left, float right, uint32_t position) {
    AnimationCurve::KeyFrame *k = key(row, col, index);
    if(k) {
        UndoManager::instance()->push(new UndoUpdateKey(row, col, index, value, left, right, position, this, "Update Key"));
    }
}

void AnimationClipModel::onInsertKey(int row, int col, int pos) {
    if(row >= 0) {
        if(!m_pClip->m_Tracks.empty()) {
            UndoManager::instance()->push(new UndoInsertKey(row, col, pos, this, "Insert Key"));
        }
    }
}

void AnimationClipModel::onRemoveKey(int row, int col, int index) {
    if(row >= 0 && index >= 0) {
        if(!m_pClip->m_Tracks.empty()) {
            UndoManager::instance()->push(new UndoRemoveKey(row, col, index, this, "Remove Key"));
        }
    }
}

void AnimationClipModel::updateController() {
    m_pController->setStateMachine(m_pStateMachine);
    m_pController->setPosition(1000 * uint32_t(m_Position));
    m_pController->setClip(m_pClip);

    emit changed();

    emit layoutAboutToBeChanged();
    emit layoutChanged();
}

void UndoInsertKey::undo() {
    auto &curves = (*std::next(m_pModel->clip()->m_Tracks.begin(), m_Row)).curves();

    int i = (m_Column == -1) ? 0 : m_Column;
    for(auto index : m_Indices) {
        auto &curve = curves[i];
        auto it = std::next(curve.m_Keys.begin(), index);

        curve.m_Keys.erase(it);
        i++;
    }

    m_pModel->updateController();
}

void UndoInsertKey::redo() {
    m_Indices.clear();

    auto &curves = (*std::next(m_pModel->clip()->m_Tracks.begin(), m_Row)).curves();

    if(m_Column > -1) {
        auto &curve = curves[m_Column];
        insertKey(curve);
    } else {
        for(int i = 0; i < curves.size(); i++) {
            auto &curve = curves[i];
            insertKey(curve);
        }
    }

    m_pModel->updateController();
}

void UndoInsertKey::insertKey(AnimationCurve &curve) {
    AnimationCurve::KeyFrame key;
    key.m_Position = uint32_t(m_Position);

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
    m_Indices.push_back(index);
    curve.m_Keys.insert(curve.m_Keys.begin() + index, key);
}

void UndoRemoveKey::undo() {
    auto &curves = (*std::next(m_pModel->clip()->m_Tracks.begin(), m_Row)).curves();
    auto &curve = curves[m_Column];

    curve.m_Keys.insert(curve.m_Keys.begin() + m_Index, m_Key);

    m_pModel->updateController();
}

void UndoRemoveKey::redo() {
    auto &curves = (*std::next(m_pModel->clip()->m_Tracks.begin(), m_Row)).curves();
    auto &curve = curves[m_Column];
    auto it = std::next(curve.m_Keys.begin(), m_Index);

    m_Key = *it;

    curve.m_Keys.erase(it);

    m_pModel->updateController();
}

void UndoUpdateKey::undo() {
    AnimationCurve::KeyFrame *k = m_pModel->key(m_Row, m_Column, m_Index);
    if(k) {
        *k = m_Key;

        m_pModel->updateController();
    }
}

void UndoUpdateKey::redo() {
    AnimationCurve::KeyFrame *k = m_pModel->key(m_Row, m_Column, m_Index);
    if(k) {
        m_Key = *k;

        k->m_Value = m_Value;
        k->m_LeftTangent = m_Left;
        k->m_RightTangent = m_Right;
        k->m_Position = m_Position;

        m_pModel->updateController();
    }
}

void UndoUpdateKeys::undo() {
    redo();
}

void UndoUpdateKeys::redo() {
    QVariant save = m_pModel->trackData(m_Row);

    auto &curves = (*std::next(m_pModel->clip()->m_Tracks.begin(), m_Row)).curves();

    curves.clear();

    foreach(QVariant it, m_Data.toList()) {
        QVariantList t = it.toList();

        int32_t component = t[0].toInt();
        AnimationCurve curve;

        for(int32_t i = 1; i < t.size(); i++) {
            QVariantList k = t[i].toList();

            AnimationCurve::KeyFrame key;
            key.m_Position = k[0].toUInt();
            key.m_Type = AnimationCurve::KeyFrame::Type(k[1].toUInt());
            key.m_Value = k[2].toFloat();
            key.m_LeftTangent = k[3].toFloat();
            key.m_RightTangent = k[4].toFloat();

            curve.m_Keys.push_back(key);
        }
        curves[component] = curve;
    }
    m_Data = save;

    m_pModel->updateController();
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
