#ifndef ANIMATIONCLIP_H
#define ANIMATIONCLIP_H

#include "resource.h"

#include <variantanimation.h>

class ENGINE_EXPORT Motion {
public:
    Motion();

    virtual int duration() const;

    virtual Vector4 valueVector4(float time) const;

    virtual Quaternion valueQuaternion(float time) const;

    virtual TString valueString(float time) const;

    TString path() const;
    void setPath(const TString &path);

    TString property() const;
    void setProperty(const TString &property);

    int hash() const;

protected:
    TString m_path;

    TString m_property;

    int m_hash;

};

class ENGINE_EXPORT AnimationTrack : public Motion {
    A_PROPERTIES(
        A_PROPERTY(TString, path, AnimationTrack::path, AnimationTrack::setPath),
        A_PROPERTY(TString, property, AnimationTrack::property, AnimationTrack::setProperty),
        A_PROPERTY(int, duration, AnimationTrack::duration, AnimationTrack::setDuration)
    )
    A_METHODS(
        A_METHOD(int, AnimationTrack::hash),
        A_METHOD(void, AnimationTrack::fixCurves)
    )

public:
    struct Frame {
        TString m_value;

        float m_position = 0.0f;
    };
    typedef std::vector<Frame> Frames;

public:
    AnimationTrack();

    int duration() const override;
    void setDuration(int duration);

    void fixCurves();

    Vector4 valueVector4(float time) const override;

    Quaternion valueQuaternion(float time) const override;

    TString valueString(float time) const override;

    AnimationCurve &curve();

    Frames &frames();

    Variant toVariant() const;
    void fromVariant(const Variant &variant);

private:
    AnimationCurve m_curve;

    Frames m_frames;

    int m_duration;

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

    int addAnimationTrack(const AnimationTrack &track);
    void removeAnimationTrack(int index);

    AnimationTrackList &tracks();

    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

protected:
    AnimationTrackList m_tracks;

};

#endif // ANIMATIONCLIP_H
