#include "resources/animationclip.h"

#define TRACKS  "Tracks"

void AnimationClip::loadUserData(const VariantMap &data) {
    PROFILE_FUNCTION()

    {
        auto section = data.find(TRACKS);
        if(section != data.end()) {
            for(auto t : (*section).second.value<VariantList>()) {
                VariantList tracks  = t.toList();
                auto i          = tracks.begin();
                Track track;
                track.path      = (*i).toString();
                i++;
                track.property  = (*i).toString();
                i++;
                for(auto c : (*i).toList()) {
                    VariantList keys    = c.toList();
                    auto k      = keys.begin();
                    KeyFrame key;
                    key.mPosition   = static_cast<uint32_t>((*k).toInt());
                    k++;
                    key.mType       = static_cast<KeyFrame::Type>((*k).toInt());
                    k++;
                    key.mValue      = (*k);
                    k++;
                    key.mSupport    = (*k);
                    track.curve.push_back(key);
                }
                m_Tracks.push_back(track);
            }
        }
    }
}

bool AnimationClip::compare(const KeyFrame &first, const KeyFrame &second) {
    return ( first.mPosition < second.mPosition );
}
