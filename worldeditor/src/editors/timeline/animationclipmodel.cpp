#include "animationclipmodel.h"

#include <resources/animationclip.h>
#include <components/actor.h>

#include "assetmanager.h"

bool compareTracks(const AnimationTrack &first, const AnimationTrack &second) {
    if(first.path() == second.path()) {
        return first.property() < second.property();
    }
    return first.path() < second.path();
}

AnimationClipModel::AnimationClipModel(QObject *parent) :
        QAbstractItemModel(parent),
        m_clip(nullptr),
        m_rootActor(nullptr),
        m_clipSettings(nullptr) {

}

uint32_t AnimationClipModel::findNear(uint32_t current, bool backward) {
    uint32_t result = 0;
    if(m_clip) {
        if(backward) {
            result = 0;
            for(auto it : m_clip->m_Tracks) {
                for(auto &c : it.curves()) {
                    auto key = c.second.m_Keys.rbegin();
                    while(key != c.second.m_Keys.rend()) {
                        float pos = key->m_Position * m_clip->duration();
                        if(pos < current) {
                            result = MAX(result, pos);
                            break;
                        }
                        key++;
                    }
                }
            }
        } else {
            result = m_clip->duration();
            for(auto it : m_clip->m_Tracks) {
                for(auto &c : it.curves()) {
                    for(auto key : c.second.m_Keys) {
                        float pos = key.m_Position * m_clip->duration();
                        if(pos > current) {
                            result = MIN(result, pos);
                            break;
                        }
                    }
                }
            }
        }
    }
    return result;
}

void AnimationClipModel::setClip(AnimationClip *clip, Actor *root) {
    m_clip = clip;
    m_rootActor = root;
    if(m_clip) {
        string guid = Engine::reference(m_clip);
        string path = AssetManager::instance()->guidToPath(guid);
        m_clipSettings = AssetManager::instance()->fetchSettings(QString(path.c_str()));
    }
    emit layoutAboutToBeChanged();
    emit layoutChanged();
}

QVariant AnimationClipModel::data(const QModelIndex &index, int role) const {
    if(!index.isValid() || m_clip == nullptr) {
        return QVariant();
    }

    switch(role) {
        case Qt::EditRole:
        case Qt::ToolTipRole:
        case Qt::DisplayRole: {
            auto it = m_clip->m_Tracks.begin();
            if(index.internalPointer() == &m_clip->m_Tracks) {
                advance(it, index.row());
                if(it != m_clip->m_Tracks.end()) {
                    QStringList lst = QString::fromStdString(it->path()).split('/');
                    lst.pop_back();
                    QString actor;
                    if(lst.isEmpty()) {
                        actor = m_rootActor->name().c_str();
                    } else {
                        actor = lst.last();
                    }

                    return QString("%1 : %2").arg(actor, QString(it->property().c_str()).replace('_', ""));
                }
            } else {
                advance(it, index.parent().row());
                int component = std::next(it->curves().begin(), index.row())->first;
                if(component >= 0) {
                    static const QStringList components = {"x", "y", "z", "w"};
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
    if(m_clip) {
        AnimationTrackList *list = &m_clip->m_Tracks;
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
    if(index.isValid() && m_clip) {
        AnimationTrackList *list  = &m_clip->m_Tracks;
        if(index.internalPointer() != list) {
            int row = 0;
            for(auto &it : m_clip->m_Tracks) {
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
    if(m_clip) {
        AnimationTrackList *list = &m_clip->m_Tracks;
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
        if(m_clip->m_Tracks.size() > size_t(track)) {
            AnimationTrack &t = *std::next(m_clip->m_Tracks.begin(), track);
            if(t.curves().size() > col && t.curves()[col].m_Keys.size() > index) {
                return &t.curves()[col].m_Keys[index];
            }
        }
    }
    return nullptr;
}

AnimationTrack &AnimationClipModel::track(int32_t track) {
    return *std::next(m_clip->m_Tracks.begin(), track);
}

QString AnimationClipModel::targetPath(QModelIndex &index) const {
    if(m_clip) {
        auto it = m_clip->m_Tracks.begin();
        if(index.internalPointer() == &m_clip->m_Tracks) {
            advance(it, index.row());
            if(it != m_clip->m_Tracks.end()) {
                return QString::fromStdString(it->path());
            }
        }
    }
    return QString();
}

void AnimationClipModel::commitKey(int row, int col, int index, float value, float left, float right, uint32_t position) {
    AnimationCurve::KeyFrame *k = key(row, col, index);
    if(!isReadOnly() && k) {
        UndoManager::instance()->push(new UndoUpdateKey(row, col, index, value, left, right, position, this, tr("Update Keyframe")));
    }
}

bool AnimationClipModel::isReadOnly() const {
    return (m_clipSettings) ? m_clipSettings->isReadOnly() : false;
}

void AnimationClipModel::propertyUpdated(Object *object, const QString &path, const QString &property, uint32_t position) {
    const MetaObject *meta = object->metaObject();
    int32_t index = meta->indexOfProperty(qPrintable(property));
    if(index >= 0) {
        MetaProperty p = meta->property(index);
        Variant value = p.read(object);

        vector<float> data;
        switch(value.type()) {
            case MetaType::VECTOR2: {
                Vector2 v = value.toVector2();
                data = {v.x, v.y};
            } break;
            case MetaType::VECTOR3: {
                Vector3 v = value.toVector3();
                data = {v.x, v.y, v.z};
            } break;
            case MetaType::VECTOR4: {
                Vector4 v = value.toVector4();
                data = {v.x, v.y, v.z, v.w};
            } break;
            default: {
                data = {value.toFloat()};
            } break;
        }

        AnimationTrackList tracks = m_clip->m_Tracks;

        for(uint32_t component = 0; component < data.size(); component++) {
            bool createTrack = true;

            float value = data[component];

            for(auto &it : tracks) {
                if(it.path() == path.toStdString() && it.property() == property.toStdString()) {
                    bool updateKeyframe = false;

                    auto &curve = it.curves()[component];
                    for(auto &k : curve.m_Keys) {
                        if(uint32_t(k.m_Position * it.duration()) == position) {
                            k.m_Value = value;
                            k.m_LeftTangent = value;
                            k.m_RightTangent = value;
                            updateKeyframe = true;
                        }
                    }
                    if(!updateKeyframe) { // Create new keyframe
                        AnimationCurve::KeyFrame key;
                        int duration = it.duration();
                        if(duration <= position) {
                            duration = position;
                            it.setDuration(duration);
                        }
                        key.m_Position = (float)position / float(duration == 0 ? 1.0f : duration);
                        key.m_Value = value;
                        key.m_LeftTangent = key.m_Value;
                        key.m_RightTangent = key.m_Value;

                        curve.m_Keys.push_back(key);

                        it.fixCurves();
                    }
                    createTrack = false;

                    break;
                }
            }

            if(createTrack) {
                AnimationTrack track;
                track.setPath(path.toStdString());
                track.setProperty(property.toStdString());
                track.setDuration(position);

                AnimationCurve curve;

                AnimationCurve::KeyFrame key;
                key.m_Position = (position == 0 ? 0.0f : 1.0f);
                key.m_Value = value;
                key.m_LeftTangent = key.m_Value;
                key.m_RightTangent = key.m_Value;

                curve.m_Keys.push_back(key);

                track.curves()[component] = curve;

                tracks.push_back(track);
                tracks.sort(compareTracks);
            }
        }
        UndoManager::instance()->push(new UndoUpdateItems(tracks, this, tr("Update Properties")));
    }
}

void AnimationClipModel::updateController() {
    emit changed();

    emit layoutAboutToBeChanged();
    emit layoutChanged();
}

void UndoUpdateKey::undo() {
    AnimationCurve::KeyFrame *k = m_model->key(m_Row, m_Column, m_Index);
    if(k) {
        *k = m_Key;

        m_model->updateController();
    }
}

void UndoUpdateKey::redo() {
    AnimationTrack &track = (*std::next(m_model->clip()->m_Tracks.begin(), m_Row));

    AnimationCurve::KeyFrame *k = &track.curves()[m_Column].m_Keys[m_Index];
    if(k) {
        m_Key = *k;

        k->m_Value = m_Value;
        k->m_LeftTangent = m_Left;
        k->m_RightTangent = m_Right;
        k->m_Position = (float)m_Position / (float)track.duration();

        m_model->updateController();
    }
}

void UndoRemoveItems::undo() {
    int i = 0;
    for(auto &track : m_Tracks) {
        auto it = std::next(m_model->clip()->m_Tracks.begin(), m_Rows.at(i));
        m_model->clip()->m_Tracks.insert(it, track);
        i++;
    }
    m_model->updateController();
}

void UndoRemoveItems::redo() {
    m_Tracks.clear();
    for(int row : m_Rows) {
        auto it = m_model->clip()->m_Tracks.begin();
        advance(it, row);

        m_Tracks.push_back(*it);

        m_model->clip()->m_Tracks.erase(it);
    }
    m_model->updateController();
}

void UndoUpdateItems::undo() {
    redo();
}

void UndoUpdateItems::redo() {
    AnimationClip *clip = m_model->clip();
    if(clip) {
        AnimationTrackList save = clip->m_Tracks;
        clip->m_Tracks = m_Tracks;
        m_Tracks = save;
        m_model->updateController();
        emit m_model->rebind();
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
