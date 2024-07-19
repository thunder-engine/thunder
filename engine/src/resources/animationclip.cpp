#include "resources/animationclip.h"

namespace  {
    const char *gTracks = "Tracks";
}

static std::hash<std::string> hash_str;

/*!
    \class AnimationTrack
    \brief A structure that describes an animation track that will affect an object's property.
    \inmodule Resources
*/

AnimationTrack::AnimationTrack() :
        m_hash(0),
        m_duration(0) {

}
/*!
    Returns a path to the object in the hierarchy.
*/
std::string AnimationTrack::path() const {
    return m_path;
}
/*!
    Sets a \a path to the object in the hierarchy.
*/
void AnimationTrack::setPath(const std::string path) {
    m_path = path;
    m_hash = hash_str(m_path + "." + m_property);
}
/*!
    Returns a property name that will be animated.
*/
std::string AnimationTrack::property() const {
    return m_property;
}
/*!
    Sets a \a property name that will be animated.
*/
void AnimationTrack::setProperty(const std::string property) {
    m_property = property;
    m_hash = hash_str(m_path + "." + m_property);
}
/*!
    Returns a duration of track in milliseconds.
*/
int AnimationTrack::duration() const {
    return m_duration;
}
/*!
    Sets a \a duration of track in milliseconds.
*/
void AnimationTrack::setDuration(int duration) {
    m_duration = duration;
}
/*!
    Returns a hash of path and name for quick access.
*/
int AnimationTrack::hash() const {
    return m_hash;
}
/*!
    Tries to fix animation curves in the animation track. Renormalizes existant keyframes and checks the duration.
*/
void AnimationTrack::fixCurves() {
    float scale = -1.0f;
    // Sort keys
    for(uint32_t j = 0; j < (m_curve.m_keys.size() - 1); j++) {
        bool swapped = false;
        for(uint32_t i = 0; i < (m_curve.m_keys.size() - 1 - j); i++) {
            if(m_curve.m_keys[i].m_position > m_curve.m_keys[i + 1].m_position) {
                std::swap(m_curve.m_keys[i + 1], m_curve.m_keys[i]);
                swapped = true;
            }
        }
        if(!swapped) {
            break;
        }
    }

    auto &last = m_curve.m_keys.back();
    scale = MAX(scale, last.m_position);

    setDuration(duration() * scale);

    if(scale > 0.0f) {
        for(auto &it : m_curve.m_keys) {
            it.m_position /= scale;
        }
    }
}
/*!
    \internal
*/
AnimationCurve &AnimationTrack::curve() {
    return m_curve;
}

/*!
    \class AnimationClip
    \brief AnimationClip resource contains keyframe based animation data.
    \inmodule Resources

    An AnimationClip resource contains keyframe based animation data.
    The animation data split to a number of tracks that must be connected with the properties of Components.
    Each track can contain multiple curves or channels like X, Y and Z
    Which allows them to animate elements independently.
*/

/*!
    \internal
*/
void AnimationClip::loadUserData(const VariantMap &data) {
    PROFILE_FUNCTION();

    m_tracks.clear();

    auto section = data.find(gTracks);
    if(section != data.end()) {
        for(auto &trackIt : *(reinterpret_cast<VariantList *>((*section).second.data()))) {
            VariantList &trackData = *(reinterpret_cast<VariantList *>(trackIt.data()));
            auto i = trackData.begin();

            AnimationTrack track;
            track.setPath((*i).toString());
            i++;
            track.setProperty((*i).toString());
            i++;
            track.setDuration((*i).toInt());
            i++;

            AnimationCurve &curve = track.curve();

            for(auto &curveIt : *(reinterpret_cast<VariantList *>((*i).data()))) {
                VariantList &keyList = *(reinterpret_cast<VariantList *>(curveIt.data()));
                auto k = keyList.begin();

                AnimationCurve::KeyFrame key;
                key.m_position = (*k).toFloat();
                k++;
                key.m_type = static_cast<AnimationCurve::KeyFrame::Type>((*k).toInt());
                k++;
                key.m_value = (*k);
                k++;
                key.m_leftTangent = (*k).toFloat();
                k++;
                key.m_rightTangent = (*k).toFloat();

                curve.m_keys.push_back(key);
            }

            m_tracks.push_back(track);
        }
    }
}
/*!
    \internal
*/
VariantMap AnimationClip::saveUserData() const {
    VariantMap result;

    VariantList tracks;
    for(auto t : m_tracks) {
        VariantList track;
        track.push_back(t.path());
        track.push_back(t.property());
        track.push_back(t.duration());

        VariantList curve;

        for(auto &it : t.curve().m_keys) {
            VariantList key;
            key.push_back(it.m_position);
            key.push_back(it.m_type);
            key.push_back(it.m_value);
            key.push_back(it.m_leftTangent);
            key.push_back(it.m_rightTangent);

            curve.push_back(key);
        }

        track.push_back(curve);

        tracks.push_back(track);
    }

    result[gTracks] = tracks;
    return result;
}

/*!
    Returns duration of the animation clip in milliseconds.
*/
int AnimationClip::duration() const {
    PROFILE_FUNCTION();

    int32_t result = 0;
    for(auto &track : m_tracks) {
        result = MAX(track.duration(), result);
    }
    return result;
}
