#ifndef ANIMATIONCLIP_H
#define ANIMATIONCLIP_H

#include "variantanimation.h"

class NEXT_LIBRARY_EXPORT AnimationClip : public Object {
    A_REGISTER(AnimationClip, Object, Resources)

public:
   struct Track {
       string                   path;

       string                   property;

       VariantAnimation::Curve  curve;
   };
   typedef list<Track>          TrackList;

public:
    void                        loadUserData        (const VariantMap &data);

    static bool                 compare             (const KeyFrame &first, const KeyFrame &second);

public:
    TrackList                   m_Tracks;

};

#endif // ANIMATIONCLIP_H
