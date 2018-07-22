#ifndef VARIANTANIMATION_H
#define VARIANTANIMATION_H

#include "animation.h"

class VariantAnimationPrivate;

typedef pair<float, Variant>    KeyFrame;
typedef vector<KeyFrame>        FrameVector;

class NEXT_LIBRARY_EXPORT VariantAnimation : public Animation {
    A_REGISTER(VariantAnimation, Animation, Animation)

public:
    VariantAnimation                ();

    ~VariantAnimation               ();

    int32_t                         loopDuration                () const;
    void                            setLoopDuration             (int32_t duration);

    Variant                         currentValue                () const;
    virtual void                    setCurrentValue             (const Variant &value);

    FrameVector                    &keyFrames                   () const;
    void                            setKeyFrames                (const FrameVector &frames);

protected:
    void                            update                      ();

private:
    VariantAnimationPrivate        *p_ptr;
};

#endif // VARIANTANIMATION_H
