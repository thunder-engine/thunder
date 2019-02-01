#include "resources/animationclip.h"

#define TRACKS  "Tracks"

void AnimationClip::loadUserData(const VariantMap &data) {
    PROFILE_FUNCTION()
    {
        auto section = data.find(TRACKS);
        if(section != data.end()) {
            for(auto it : (*section).second.value<VariantList>()) {
                VariantList trackList = it.toList();
                auto i = trackList.begin();

                Track track;
                track.path = (*i).toString();
                i++;
                track.property = (*i).toString();
                i++;

                for(auto it : (*i).toList()) {
                    VariantList curveList = it.toList();
                    auto t = curveList.begin();

                    int32_t component = (*t).toInt();
                    t++;

                    AnimationCurve curve;
                    while(t != curveList.end()) {
                        VariantList keyList = (*t).toList();
                        auto k = keyList.begin();

                        AnimationCurve::KeyFrame key;
                        key.m_Position = static_cast<uint32_t>((*k).toInt());
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
                    track.curves[component] = curve;
                }
                m_Tracks.push_back(track);
            }
        }
    }
}

bool AnimationClip::compare(const AnimationCurve::KeyFrame &first, const AnimationCurve::KeyFrame &second) {
    return ( first.m_Position < second.m_Position );
}
