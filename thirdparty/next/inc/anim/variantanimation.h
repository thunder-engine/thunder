#ifndef VARIANTANIMATION_H
#define VARIANTANIMATION_H

#include "animation.h"

#include "keyframe.h"

class VariantAnimationPrivate;

class NEXT_LIBRARY_EXPORT VariantAnimation : public Animation {
    A_REGISTER(VariantAnimation, Animation, Animation)

public:

    typedef list<KeyFrame>          Curve;

public:
    VariantAnimation                ();

    ~VariantAnimation               ();

    int32_t                         loopDuration                () const;

    Variant                         currentValue                () const;
    virtual void                    setCurrentValue             (const Variant &value);

    Curve                          &keyFrames                   () const;
    void                            setKeyFrames                (Curve &frames);

protected:
    void                            update                      ();

private:
    VariantAnimationPrivate        *p_ptr;
};

#endif // VARIANTANIMATION_H
