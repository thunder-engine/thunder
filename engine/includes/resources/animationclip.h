#ifndef ANIMATIONCLIP_H
#define ANIMATIONCLIP_H

#include "resource.h"

#include <variantanimation.h>

class ENGINE_EXPORT AnimationTrack {

    A_PROPERTIES(
        A_PROPERTY(TString, path, AnimationTrack::path, AnimationTrack::setPath),
        A_PROPERTY(TString, property, AnimationTrack::property, AnimationTrack::setProperty),
        A_PROPERTY(int, duration, AnimationTrack::duration, AnimationTrack::setDuration)
    )
    A_METHODS(
        A_METHOD(int, AnimationTrack::hash)
    )

public:
    AnimationTrack();

    TString path() const;
    void setPath(const TString path);

    TString property() const;
    void setProperty(const TString property);

    int duration() const;
    void setDuration(int duration);

    int hash() const;

    void fixCurves();

    AnimationCurve &curve();

private:
    TString m_path;

    TString m_property;

    int m_hash;

    int m_duration;

    AnimationCurve m_curve;

};
typedef std::list<AnimationTrack> AnimationTrackList;

class ENGINE_EXPORT AnimationClip : public Resource {
    A_OBJECT(AnimationClip, Resource, Resources)

    A_NOPROPERTIES()
    A_METHODS(
        A_METHOD(int, AnimationClip::duration)
    )

public:
    int duration() const;

public:
    AnimationTrackList m_tracks;

    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

};

#endif // ANIMATIONCLIP_H
