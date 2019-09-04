#ifndef BLENDERANIMATION_H
#define BLENDERANIMATION_H

#include "animation.h"

#include "animationcurve.h"

class BlenderAnimationPrivate;

class NEXT_LIBRARY_EXPORT BlenderAnimation : public Animation {
    A_REGISTER(BlenderAnimation, Animation, Animation)

public:
    BlenderAnimation                ();

    ~BlenderAnimation               ();

    int32_t                         loopDuration                () const override;

    void                            setCurrentTime              (uint32_t msecs) override;

    Variant                         defaultValue                ();
    void                            setDefaultValue             (const Variant &value);

    virtual void                    setCurrentValue             (const Variant &value);

    void                            setOffset                   (float offset);
    void                            setTransitionTime           (float time);

    AnimationCurve                 *beginCurve                  (int32_t component = 0) const;
    void                            setBeginCurve               (AnimationCurve *curve, int32_t component = 0);

    AnimationCurve                 *endCurve                    (int32_t component = 0) const;
    void                            setEndCurve                 (AnimationCurve *curve, int32_t component = 0);

private:
    BlenderAnimationPrivate        *p_ptr;

};

#endif // BLENDERANIMATION_H
