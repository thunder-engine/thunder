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

    int32_t                         loopDuration                () const;

    Variant                         currentValue                () const;
    virtual void                    setCurrentValue             (const Variant &value);

    AnimationCurve                 &curve                       (int32_t component = 0) const;
    void                            setCurve                    (AnimationCurve &curve, int32_t component = 0);

protected:
    void                            update                      ();

private:
    VariantAnimationPrivate        *p_ptr;

};

#endif // VARIANTANIMATION_H
