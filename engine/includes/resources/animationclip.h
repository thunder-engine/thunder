#ifndef ANIMATIONCLIP_H
#define ANIMATIONCLIP_H

#include "resource.h"

#include <variantanimation.h>

class ENGINE_EXPORT AnimationTrack {

    A_PROPERTIES(
        A_PROPERTY(string, path, AnimationTrack::path, AnimationTrack::setPath),
        A_PROPERTY(string, property, AnimationTrack::property, AnimationTrack::setProperty),
        A_PROPERTY(int, duration, AnimationTrack::duration, AnimationTrack::setDuration)
    )
    A_METHODS(
        A_METHOD(int, AnimationTrack::hash)
    )

    typedef map<int32_t, AnimationCurve> CurveMap;

public:
    AnimationTrack();

    string path() const;
    void setPath(const string &path);

    string property() const;
    void setProperty(const string &property);

    int duration() const;
    void setDuration(int duration);

    int hash() const;

    void fixCurves();

    CurveMap &curves();

private:
    string m_path;

    string m_property;

    int m_hash;

    int m_duration;

    CurveMap m_curves;
};
typedef list<AnimationTrack> AnimationTrackList;

class ENGINE_EXPORT AnimationClip : public Resource {
    A_REGISTER(AnimationClip, Resource, Resources)

    A_NOPROPERTIES()
    A_METHODS(
        A_METHOD(int, AnimationClip::duration)
    )

public:
    int duration() const;

public:
    AnimationTrackList m_Tracks;

    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

};

#endif // ANIMATIONCLIP_H
