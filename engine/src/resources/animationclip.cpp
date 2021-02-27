#include "resources/animationclip.h"

#define TRACKS  "Tracks"

static hash<string> hash_str;

string AnimationTrack::path() const {
    return m_Path;
}

void AnimationTrack::setPath(const string &path) {
    m_Path = path;
    m_Hash = hash_str(m_Path + "." + m_Property);
}

string AnimationTrack::property() const {
    return m_Property;
}

void AnimationTrack::setProperty(const string &property) {
    m_Property = property;
    m_Hash = hash_str(m_Path + "." + m_Property);
}

int AnimationTrack::duration() const {
    return m_Duration;
}

void AnimationTrack::setDuration(int duration) {
    m_Duration = duration;
}

int AnimationTrack::hash() const {
    return m_Hash;
}

AnimationTrack::CurveMap &AnimationTrack::curves() {
    return m_Curves;
}

/*!
    \class AnimationClip
    \brief AnimationClip resource contains keyframe based animation data.
    \inmodule Resource

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

    auto section = data.find(TRACKS);
    if(section != data.end()) {
        VariantList &tracks = *(reinterpret_cast<VariantList *>((*section).second.data()));
        for(auto it : tracks) {
            VariantList &trackData = *(reinterpret_cast<VariantList *>(it.data()));
            auto i = trackData.begin();

            AnimationTrack track;
            track.setPath((*i).toString());
            i++;
            track.setProperty((*i).toString());
            i++;
            track.setDuration((*i).toInt());
            i++;

            for(auto it : (*i).toList()) {
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

    setState(Ready);
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
        for(auto c : t.curves()) {
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

    result[TRACKS] = tracks;
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
