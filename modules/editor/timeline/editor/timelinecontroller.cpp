#include "timelinecontroller.h"

#include <resources/animationclip.h>
#include <components/actor.h>

#include <editor/assetmanager.h>

bool compareTracks(const AnimationTrack &first, const AnimationTrack &second) {
    if(first.path() == second.path()) {
        return first.property() < second.property();
    }
    return first.path() < second.path();
}

TimelineController::TimelineController(TimelineEdit *editor) :
        m_clip(nullptr),
        m_rootActor(nullptr),
        m_clipSettings(nullptr),
        m_editor(editor) {

}

uint32_t TimelineController::findNear(uint32_t current, bool backward) {
    uint32_t result = 0;
    if(m_clip) {
        if(backward) {
            result = 0;
            for(auto it : m_clip->tracks()) {
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
            for(auto it : m_clip->tracks()) {
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

void TimelineController::setClip(AnimationClip *clip) {
    m_clip = clip;

    if(m_clip) {
        TString uuid = Engine::reference(m_clip);
        TString path = AssetManager::instance()->uuidToPath(uuid);
        m_clipSettings = AssetManager::instance()->fetchSettings(path);
    }
    emit updated();
}

void TimelineController::setRoot(Actor *root) {
    m_rootActor = root;
}

void TimelineController::removeItems(const std::list<int> &rows) {
    if(!isReadOnly()) {
        undoRedo()->push(new UndoRemoveItems(rows, this, tr("Remove Properties").toStdString()));
    }
}

AnimationCurve::KeyFrame *TimelineController::key(int32_t track, int32_t index) {
    if(track >= 0) {
        if(m_clip->tracks().size() > size_t(track)) {
            AnimationTrack &t = m_clip->tracks().at(track);
            if(t.curve().m_keys.size() > index) {
                return &t.curve().m_keys[index];
            }
        }
    }
    return nullptr;
}

void TimelineController::commitKey(int row, int index, float value, float left, float right, uint32_t position) {
    AnimationCurve::KeyFrame *k = key(row, index);
    if(!isReadOnly() && k) {
        undoRedo()->push(new UndoUpdateKey(row, index, {value}, {left}, {right}, position, this, tr("Update Keyframe").toStdString()));
    }
}

bool TimelineController::isReadOnly() const {
    return (m_clipSettings) ? m_clipSettings->isReadOnly() : false;
}

void TimelineController::propertyUpdated(Object *object, const QString &path, const QString &property, uint32_t position) {
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

        AnimationTracks tracks = m_clip->tracks(); // we are working with copy

        bool createTrack = true;

        for(auto &it : tracks) {
            if(it.path() == path.toStdString() && it.property() == property.toStdString()) {
                bool updateKeyframe = false;

                AnimationCurve &curve = it.curve();
                for(auto &k : curve.m_keys) {
                    if(uint32_t(k.m_position * it.duration()) == position) {
                        k.m_value = data;
                        k.m_leftTangent = data;
                        k.m_rightTangent = data;
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
                    key.m_value = data;
                    key.m_leftTangent = key.m_value;
                    key.m_rightTangent = key.m_value;

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
            key.m_value = data;

            key.m_leftTangent = key.m_value;
            key.m_rightTangent = key.m_value;

            curve.m_keys.push_back(key);

            tracks.push_back(track);
            std::sort(tracks.begin(), tracks.end(), compareTracks);
        }

        undoRedo()->push(new UndoUpdateItems(tracks, this, tr("Update Properties").toStdString()));
    }
}

void UndoUpdateKey::undo() {
    AnimationCurve::KeyFrame *k = m_controller->key(m_row, m_index);
    if(k) {
        *k = m_key;

        m_controller->updated();
    }
}

void UndoUpdateKey::redo() {
    AnimationTrack &track = m_controller->clip()->tracks().at(m_row);

    AnimationCurve::KeyFrame *k = &track.curve().m_keys[m_index];

    m_key = *k;

    k->m_value = m_value;
    k->m_leftTangent = m_left;
    k->m_rightTangent = m_right;
    k->m_position = (float)m_position / (float)track.duration();

    m_controller->updated();
}

void UndoRemoveItems::undo() {
    int i = 0;

    AnimationTracks &tracks = m_controller->clip()->tracks();
    for(auto &track : m_tracks) {
        auto it = std::next(tracks.begin(), *std::next(m_rows.begin(), i));
        tracks.insert(it, track);
        i++;
    }
    m_controller->updated();
}

void UndoRemoveItems::redo() {
    m_tracks.clear();

    AnimationTracks &tracks = m_controller->clip()->tracks();
    for(int row : m_rows) {
        auto it = tracks.begin();
        advance(it, row);

        m_tracks.push_back(*it);

        tracks.erase(it);
    }
    m_controller->updated();
}

void UndoUpdateItems::undo() {
    redo();
}

void UndoUpdateItems::redo() {
    AnimationClip *clip = m_controller->clip();
    if(clip) {
        AnimationTracks save = clip->tracks();
        clip->tracks() = m_tracks;
        m_tracks = save;

        m_controller->updated();
        m_controller->rebind();
    }
}

void UndoInsertKey::undo() {
    AnimationCurve &curve = (*std::next(m_controller->clip()->tracks().begin(), m_row)).curve();

    for(auto index : m_indices) {
        auto it = std::next(curve.m_keys.begin(), index);

        curve.m_keys.erase(it);
    }

    m_controller->updated();
}

void UndoInsertKey::redo() {
    m_indices.clear();

    AnimationTrack &track = (*std::next(m_controller->clip()->tracks().begin(), m_row));

    insertKey(track.curve());

    m_controller->updated();
}

void UndoInsertKey::insertKey(AnimationCurve &curve) {
    AnimationCurve::KeyFrame key;
    key.m_position = m_position;
    key.m_value = { curve.valueFloat(key.m_position) };
    key.m_leftTangent = key.m_value;
    key.m_rightTangent = key.m_value;

    int index = 0;
    for(auto &it : curve.m_keys) {
        if(it.m_position > key.m_position) {
            break;
        }
        index++;
    }
    m_indices.push_back(index);
    curve.m_keys.insert(curve.m_keys.begin() + index, key);
}
