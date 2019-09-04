#ifndef VARIANTANIMATION_H
#define VARIANTANIMATION_H

#include "animation.h"

#include "animationcurve.h"

class VariantAnimationPrivate;

class NEXT_LIBRARY_EXPORT VariantAnimation : public Animation {
    A_REGISTER(VariantAnimation, Animation, Animation)

public:
    VariantAnimation                ();

    ~VariantAnimation               ();

    int32_t                         loopDuration                () const override;

    Variant                         currentValue                () const;
    void                            setCurrentValue             (const Variant &value);

    AnimationCurve                 *curve                       (int32_t component = 0) const;
    void                            setCurve                    (AnimationCurve *curve, int32_t component = 0);

    void                            setCurrentTime              (uint32_t msecs) override;

private:
    VariantAnimationPrivate        *p_ptr;

};

#endif // VARIANTANIMATION_H
