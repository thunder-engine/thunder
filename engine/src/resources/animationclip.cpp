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
    for(auto &curve : m_curves) {
        // Sort keys
        for(uint32_t j = 0; j < (curve.second.m_Keys.size() - 1); j++) {
            bool swapped = false;
            for(uint32_t i = 0; i < (curve.second.m_Keys.size() - 1 - j); i++) {
                if(curve.second.m_Keys[i].m_Position > curve.second.m_Keys[i + 1].m_Position) {
                    std::swap(curve.second.m_Keys[i + 1], curve.second.m_Keys[i]);
                    swapped = true;
                }
            }
            if(!swapped) {
                break;
            }
        }

        auto &last = curve.second.m_Keys.back();
        scale = MAX(scale, last.m_Position);
    }

    setDuration(duration() * scale);

    if(scale > 0.0f) {
        for(auto &curve : m_curves) {
            for(auto &it : curve.second.m_Keys) {
                it.m_Position /= scale;
            }
        }
    }
}
/*!
    \internal
*/
AnimationTrack::CurveMap &AnimationTrack::curves() {
    return m_curves;
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

    m_Tracks.clear();

    auto section = data.find(gTracks);
    if(section != data.end()) {
        VariantList &tracks = *(reinterpret_cast<VariantList *>((*section).second.data()));
        for(auto &it : tracks) {
            VariantList &trackData = *(reinterpret_cast<VariantList *>(it.data()));
            auto i = trackData.begin();

            AnimationTrack track;
            track.setPath((*i).toString());
            i++;
            track.setProperty((*i).toString());
            i++;
            track.setDuration((*i).toInt());
            i++;

            for(auto &it : (*i).toList()) {
                VariantList &curveList = *(reinterpret_cast<VariantList *>(it.data()));
                auto t = curveList.begin();

                int32_t component = (*t).toInt();
                t++;

                AnimationCurve curve;
                while(t != curveList.end()) {
                    VariantList &keyList = *(reinterpret_cast<VariantList *>((*t).data()));
                    auto k = keyList.begin();

                    AnimationCurve::KeyFrame key;
                    key.m_Position = (*k).toFloat();
                    k++;
                    key.m_Type = static_cast<AnimationCurve::KeyFrame::Type>((*k).toInt());
                    k++;
                    key.m_Value = (*k).toFloat();
                    k++;
                    key.m_LeftTangent = (*k).toFloat();
                    k++;
                    key.m_RightTangent = (*k).toFloat();

                    curve.m_Keys.push_back(key);

                    t++;
                }
                track.curves()[component] = curve;
            }
            m_Tracks.push_back(track);
        }
    }
}
/*!
    \internal
*/
VariantMap AnimationClip::saveUserData() const {
    VariantMap result;

    VariantList tracks;
    for(auto t : m_Tracks) {
        VariantList track;
        track.push_back(t.path());
        track.push_back(t.property());
        track.push_back(t.duration());

        VariantList curves;
        for(auto &c : t.curves()) {
            VariantList curve;
            curve.push_back(c.first);

            for(auto it : c.second.m_Keys) {
                VariantList key;
                key.push_back(it.m_Position);
                key.push_back(it.m_Type);
                key.push_back(it.m_Value);
                key.push_back(it.m_LeftTangent);
                key.push_back(it.m_RightTangent);

                curve.push_back(key);
            }
            curves.push_back(curve);
        }
        track.push_back(curves);

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
    for(auto &track : m_Tracks) {
        result = MAX(track.duration(), result);
    }
    return result;
}
