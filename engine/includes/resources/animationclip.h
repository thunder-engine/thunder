#ifndef ANIMATIONCLIP_H
#define ANIMATIONCLIP_H

#include "resource.h"

#include <variantanimation.h>

class NEXT_LIBRARY_EXPORT AnimationTrack {

    A_PROPERTIES(
        A_PROPERTY(string, path, AnimationTrack::path, AnimationTrack::setPath),
        A_PROPERTY(string, property, AnimationTrack::property, AnimationTrack::setProperty)
    )
    A_METHODS(
        A_METHOD(int, AnimationTrack::hash)
    )

    typedef map<int32_t, AnimationCurve> CurveMap;

public:
    string path() const;
    void setPath(const string &path);

    string property() const;
    void setProperty(const string &property);

    int hash() const;

    CurveMap &curves();

private:
    string m_Path;

    string m_Property;

    int m_Hash;

    CurveMap m_Curves;
};
typedef list<AnimationTrack> AnimationTrackList;

class NEXT_LIBRARY_EXPORT AnimationClip : public Resource {
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
protected:
    VariantMap saveUserData() const override;

};

#endif // ANIMATIONCLIP_H
