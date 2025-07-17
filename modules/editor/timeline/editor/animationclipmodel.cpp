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
            for(auto it : m_clip->m_tracks) {
                AnimationCurve &curve = it.curve();

                auto key = curve.m_keys.rbegin();
                while(key != curve.m_keys.rend()) {
                    uint32_t pos = static_cast<uint32_t>(key->m_position * m_clip->duration());
                    if(pos < current) {
                        result = MAX(result, pos);
                        break;
                    }
                    key++;
                }
            }
        } else {
            result = m_clip->duration();
            for(auto it : m_clip->m_tracks) {
                AnimationCurve &curve = it.curve();

                auto key = curve.m_keys.begin();
                while(key != curve.m_keys.end()) {
                    uint32_t pos = static_cast<uint32_t>(key->m_position * m_clip->duration());
                    if(pos > current) {
                        result = MIN(result, pos);
                        break;
                    }
                    key++;
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
        TString guid = Engine::reference(m_clip);
        TString path = AssetManager::instance()->guidToPath(guid);
        m_clipSettings = AssetManager::instance()->fetchSettings(QString(path.data()));
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
            auto it = m_clip->m_tracks.begin();
            if(index.internalPointer() == &m_clip->m_tracks) {
                advance(it, index.row());
                if(it != m_clip->m_tracks.end()) {
                    StringList lst = it->path().split('/');
                    lst.pop_back();
                    TString actor;
                    if(lst.empty()) {
                        actor = m_rootActor->name().data();
                    } else {
                        actor = lst.back();
                    }

                    return QString("%1 : %2").arg(actor.data(), QString(it->property().data()).replace('_', ""));
                }
            } else {
                advance(it, index.parent().row());
                static const QStringList components = {"x", "y", "z", "w"};
                return QString("%1.%2").arg(it->property().data(), components.at(index.row()));
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
        AnimationTrackList *list = &m_clip->m_tracks;
        if(!parent.isValid()) {
            return createIndex(row, column, list);
        } else {
            if(parent.internalPointer() == list) {
                if(static_cast<uint32_t>(parent.row()) < list->size()) {
                    void *ptr = &(std::next(list->begin(), parent.row())->curve());
                    return createIndex(row, column, ptr);
                }
            }
        }
    }
    return QModelIndex();
}

QModelIndex AnimationClipModel::parent(const QModelIndex &index) const {
    if(index.isValid() && m_clip) {
        AnimationTrackList *list  = &m_clip->m_tracks;
        if(index.internalPointer() != list) {
            int row = 0;
            for(auto &it : m_clip->m_tracks) {
                if(index.internalPointer() == &(it.curve())) {
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
        AnimationTrackList *list = &m_clip->m_tracks;
        if(!list->empty()) {
            if(!parent.isValid()) {
                return static_cast<int32_t>(list->size());
            } else {
                if(parent.internalPointer() == list) {
                    if(static_cast<uint32_t>(parent.row()) < list->size()) {
                        int result = 0;

                        //AnimationCurve &curve = std::next(list->begin(), parent.row())->curve();

                        return result;
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

AnimationCurve::KeyFrame *AnimationClipModel::key(int32_t track, int32_t index) {
    if(track >= 0) {
        if(m_clip->m_tracks.size() > size_t(track)) {
            AnimationTrack &t = *std::next(m_clip->m_tracks.begin(), track);
            if(t.curve().m_keys.size() > index) {
                return &t.curve().m_keys[index];
            }
        }
    }
    return nullptr;
}

AnimationTrack &AnimationClipModel::track(int32_t track) {
    return *std::next(m_clip->m_tracks.begin(), track);
}

QString AnimationClipModel::targetPath(QModelIndex &index) const {
    if(m_clip) {
        auto it = m_clip->m_tracks.begin();
        if(index.internalPointer() == &m_clip->m_tracks) {
            advance(it, index.row());
            if(it != m_clip->m_tracks.end()) {
                return QString::fromStdString(it->path().data());
            }
        }
    }
    return QString();
}

void AnimationClipModel::commitKey(int row, int index, float value, float left, float right, uint32_t position) {
    AnimationCurve::KeyFrame *k = key(row, index);
    if(!isReadOnly() && k) {
        UndoManager::instance()->push(new UndoUpdateKey(row, index, value, left, right, position, this, tr("Update Keyframe")));
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

        std::vector<float> data;
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

        AnimationTrackList tracks = m_clip->m_tracks;

        for(uint32_t component = 0; component < data.size(); component++) {
            bool createTrack = true;

            float value = data[component];

            for(auto &it : tracks) {
                if(it.path() == path.toStdString() && it.property() == property.toStdString()) {
                    bool updateKeyframe = false;

                    AnimationCurve &curve = it.curve();
                    for(auto &k : curve.m_keys) {
                        if(uint32_t(k.m_position * it.duration()) == position) {
                            k.m_value = value;
                            k.m_leftTangent = value;
                            k.m_rightTangent = value;
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
                        key.m_position = (float)position / float(duration == 0 ? 1.0f : duration);
                        key.m_value = value;
                        if(key.m_value.type() == MetaType::FLOAT) {
                            key.m_leftTangent = key.m_value.toFloat();
                            key.m_rightTangent = key.m_value.toFloat();
                        }

                        curve.m_keys.push_back(key);

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

                AnimationCurve &curve = track.curve();

                AnimationCurve::KeyFrame key;
                key.m_position = (position == 0 ? 0.0f : 1.0f);
                key.m_value = value;
                if(key.m_value.type() == MetaType::FLOAT) {
                    key.m_leftTangent = key.m_value.toFloat();
                    key.m_rightTangent = key.m_value.toFloat();
                }

                curve.m_keys.push_back(key);

                tracks.push_back(track);
                tracks.sort(compareTracks);
            }
        }
        UndoManager::instance()->push(new UndoUpdateItems(tracks, this, tr("Update Properties")));
    }
}

void AnimationClipModel::updateController() {
    emit layoutAboutToBeChanged();
    emit layoutChanged();
}

void UndoUpdateKey::undo() {
    AnimationCurve::KeyFrame *k = m_model->key(m_row, m_index);
    if(k) {
        *k = m_key;

        m_model->updateController();
    }
}

void UndoUpdateKey::redo() {
    AnimationTrack &track = (*std::next(m_model->clip()->m_tracks.begin(), m_row));

    AnimationCurve::KeyFrame *k = &track.curve().m_keys[m_index];

    m_key = *k;

    k->m_value = m_value;
    k->m_leftTangent = m_left;
    k->m_rightTangent = m_right;
    k->m_position = (float)m_position / (float)track.duration();

    m_model->updateController();
}

void UndoRemoveItems::undo() {
    int i = 0;
    for(auto &track : m_tracks) {
        auto it = std::next(m_model->clip()->m_tracks.begin(), m_rows.at(i));
        m_model->clip()->m_tracks.insert(it, track);
        i++;
    }
    m_model->updateController();
}

void UndoRemoveItems::redo() {
    m_tracks.clear();
    for(int row : m_rows) {
        auto it = m_model->clip()->m_tracks.begin();
        advance(it, row);

        m_tracks.push_back(*it);

        m_model->clip()->m_tracks.erase(it);
    }
    m_model->updateController();
}

void UndoUpdateItems::undo() {
    redo();
}

void UndoUpdateItems::redo() {
    AnimationClip *clip = m_model->clip();
    if(clip) {
        AnimationTrackList save = clip->m_tracks;
        clip->m_tracks = m_tracks;
        m_tracks = save;
        m_model->updateController();
        emit m_model->rebind();
    }
}

void UndoInsertKey::undo() {
    AnimationCurve &curve = (*std::next(m_model->clip()->m_tracks.begin(), m_row)).curve();

    for(auto index : m_indices) {
        auto it = std::next(curve.m_keys.begin(), index);

        curve.m_keys.erase(it);
    }

    m_model->updateController();
}

void UndoInsertKey::redo() {
    m_indices.clear();

    AnimationTrack &track = (*std::next(m_model->clip()->m_tracks.begin(), m_row));

    insertKey(track.curve());

    m_model->updateController();
}

void UndoInsertKey::insertKey(AnimationCurve &curve) {
    AnimationCurve::KeyFrame key;
    key.m_position = m_position;
    key.m_value = curve.value(key.m_position);
    if(key.m_value.type() == MetaType::FLOAT) {
        key.m_leftTangent = key.m_value.toFloat();
        key.m_rightTangent = key.m_value.toFloat();
    }

    int index = 0;
    for(auto it : curve.m_keys) {
        if(it.m_position > key.m_position) {
            break;
        }
        index++;
    }
    m_indices.push_back(index);
    curve.m_keys.insert(curve.m_keys.begin() + index, key);
}
