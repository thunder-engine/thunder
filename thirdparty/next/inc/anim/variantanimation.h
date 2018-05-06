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
    void                            setLoopDuration             (int32_t msec);

    Variant                         currentValue                () const;

    void                            setKeyFrames                (const FrameVector &frames);

protected:
    void                            update                      ();

    virtual void                    valueUpdated                (const Variant &value);
private:
    VariantAnimationPrivate        *p_ptr;
};

#endif // VARIANTANIMATION_H
