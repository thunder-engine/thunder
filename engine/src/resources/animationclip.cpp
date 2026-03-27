#include "resources/animationclip.h"

namespace  {
    const char *gTracks("Tracks");
}

Motion::Motion() :
    m_hash(0) {

}

int Motion::duration() const {
    return 0;
}

Vector4 Motion::valueVector4(float time) const {
    return Vector4();
}

Quaternion Motion::valueQuaternion(float time) const {
    return Quaternion();
}

TString Motion::valueString(float time) const {
    return TString();
}
/*!
    Returns a path to the object in the hierarchy.
*/
TString Motion::path() const {
    return m_path;
}
/*!
    Sets a \a path to the object in the hierarchy.
*/
void Motion::setPath(const TString &path) {
    m_path = path;
    m_hash = Mathf::hashString(m_path + "." + m_property);
}
/*!
    Returns a property name that will be animated.
*/
TString Motion::property() const {
    return m_property;
}
/*!
    Sets a \a property name that will be animated.
*/
void Motion::setProperty(const TString &property) {
    m_property = property;
    m_hash = Mathf::hashString(m_path + "." + m_property);
}
/*!
    Returns a hash of path and name for quick access.
*/
int Motion::hash() const {
    return m_hash;
}

/*!
    \class AnimationTrack
    \brief A structure that describes an animation track that will affect an object's property.
    \inmodule Resources
*/

AnimationTrack::AnimationTrack() :
    m_duration(0) {

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
    Tries to fix animation curves in the animation track. Renormalizes existant keyframes and checks the duration.
*/
void AnimationTrack::fixCurves() {
    PROFILE_FUNCTION();

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

    const auto &last = m_curve.m_keys.back();
    scale = MAX(scale, last.m_position);

    if(scale > 0.0f) {
        setDuration(scale);

        for(auto &it : m_curve.m_keys) {
            it.m_position /= scale;
        }
    }
}
/*!
    Returns current value for the animation curve.
    Parameter normalized \a time is used to interpolate value between key frames.
*/
Vector4 AnimationTrack::valueVector4(float time) const {
    PROFILE_FUNCTION();

    return m_curve.valueVector4(time);
}
/*!
    Returns current value for the animation curve.
    Parameter normalized \a time is used to interpolate value between key frames.
*/
Quaternion AnimationTrack::valueQuaternion(float time) const {
    PROFILE_FUNCTION();

    return m_curve.valueQuaternion(time);
}
/*!
    Returns current value at normalized \a time position.
*/
TString AnimationTrack::valueString(float time) const {
    PROFILE_FUNCTION();

    int32_t b = -1;
    for(uint32_t i = 0; i < m_frames.size(); i++) {
        if(time >= m_frames[i].m_position) {
            b = i;
        } else {
            break;
        }
    }

    if(b > -1) {
        return m_frames[b].m_value;
    }

    return TString();
}
/*!
    Returns curve used for interpolation based animation.
*/
AnimationCurve &AnimationTrack::curve() {
    return m_curve;
}
/*!
    Returns set of frames for frame-by-frame animation (e.g. sprites).
*/
AnimationTrack::Frames &AnimationTrack::frames() {
    return m_frames;
}
/*!
    Serializes current track to Variant.
*/
Variant AnimationTrack::AnimationTrack::toVariant() const {
    PROFILE_FUNCTION();

    VariantList track;
    track.push_back(path());
    track.push_back(property());
    track.push_back(duration());

    VariantList curve;
    for(auto &it : m_curve.m_keys) {
        VariantList key;
        key.push_back(it.m_position);
        key.push_back(it.m_type);

        for(int32_t i = 0; i < it.m_value.size(); i++) {
            key.push_back(it.m_value[i]);
            if(it.m_type == AnimationCurve::KeyFrame::Cubic) {
                key.push_back(it.m_leftTangent[i]);
                key.push_back(it.m_rightTangent[i]);
            }
        }

        curve.push_back(key);
    }
    track.push_back(curve);

    if(!m_frames.empty()) {
        VariantList frames;
        for(auto &it : m_frames) {
            VariantList key;
            key.push_back(it.m_position);
            key.push_back(it.m_value);

            frames.push_back(key);
        }
        track.push_back(frames);
    }

    return track;
}
/*!
    Deserializes current track from \a variant.
*/
void AnimationTrack::fromVariant(const Variant &variant) {
    PROFILE_FUNCTION();

    VariantList &trackData = *(reinterpret_cast<VariantList *>(variant.data()));
    auto i = trackData.begin();

    setPath((*i).toString());
    i++;
    setProperty((*i).toString());
    i++;
    setDuration((*i).toInt());
    i++;

    m_curve.m_keys.clear();

    for(auto &curveIt : *(reinterpret_cast<VariantList *>((*i).data()))) {
        VariantList &keyList = *(reinterpret_cast<VariantList *>(curveIt.data()));
        auto k = keyList.begin();

        AnimationCurve::KeyFrame key;
        key.m_position = (*k).toFloat();
        if(!std::isfinite(key.m_position)) {
            key.m_position = 0.0f;
        }
        k++;
        key.m_type = static_cast<AnimationCurve::KeyFrame::Type>((*k).toInt());
        k++;

        while(k != keyList.end()) {
            key.m_value.push_back((*k).toFloat());
            k++;
            if(key.m_type == AnimationCurve::KeyFrame::Cubic) {
                key.m_leftTangent.push_back((*k).toFloat());
                k++;
                key.m_rightTangent.push_back((*k).toFloat());
                k++;
            }
        }

        m_curve.m_keys.push_back(key);
    }
    i++;

    m_frames.clear();

    if(i != trackData.end()) {
        for(auto &frameIt : *(reinterpret_cast<VariantList *>((*i).data()))) {
            VariantList &frameList = *(reinterpret_cast<VariantList *>(frameIt.data()));
            auto f = frameList.begin();

            Frame frame;
            frame.m_position = (*f).toFloat();
            if(!std::isfinite(frame.m_position)) {
                frame.m_position = 0.0f;
            }
            f++;
            frame.m_value = (*f).toString();

            m_frames.push_back(frame);
        }
    }
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
            AnimationTrack track;
            track.fromVariant(trackIt);

            addAnimationTrack(track);
        }
    }
}
/*!
    \internal
*/
VariantMap AnimationClip::saveUserData() const {
    PROFILE_FUNCTION();

    VariantMap result;

    VariantList tracks;
    for(const AnimationTrack &t : m_tracks) {
        tracks.push_back(t.toVariant());
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
    for(const AnimationTrack &track : m_tracks) {
        result = MAX(track.duration(), result);
    }
    return result;
}
/*!
    Adds animation \a track to current AnimationClip.
    Returns index of added track;
*/
int AnimationClip::addAnimationTrack(const AnimationTrack &track) {
    PROFILE_FUNCTION();

    m_tracks.push_back(track);

    return m_tracks.size() - 1;
}
/*!
    Removes animation track at givven \a index.
*/
void AnimationClip::removeAnimationTrack(int index) {
    PROFILE_FUNCTION();

    auto it = std::next(m_tracks.begin(), index);
    if(it != m_tracks.end()) {
        m_tracks.erase(it);
    }
}
/*!
    Returns all tracks associated with current animation clip.
*/
AnimationTracks &AnimationClip::tracks() {
    return m_tracks;
}
