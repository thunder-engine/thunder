#ifndef ANIMATIONCLIP_H
#define ANIMATIONCLIP_H

#include "resource.h"

#include <variantanimation.h>

class NEXT_LIBRARY_EXPORT AnimationClip : public Resource {
    A_REGISTER(AnimationClip, Resource, Resources)

public:
   struct Track {
       string path;

       string property;

       uint32_t hash;

       map<int32_t, AnimationCurve> curves;
   };
   typedef list<Track> TrackList;

public:
    uint32_t duration() const;

    static bool compare(const AnimationCurve::KeyFrame &first, const AnimationCurve::KeyFrame &second);

    void loadUserData(const VariantMap &data) override;

public:
    TrackList m_Tracks;

};

#endif // ANIMATIONCLIP_H
