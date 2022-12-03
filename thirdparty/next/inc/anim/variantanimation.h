#ifndef VARIANTANIMATION_H
#define VARIANTANIMATION_H

#include "animation.h"

#include "animationcurve.h"

class NEXT_LIBRARY_EXPORT VariantAnimation : public Animation {
    A_REGISTER(VariantAnimation, Animation, Animation)

public:
    VariantAnimation();

    ~VariantAnimation();

    int32_t duration() const override;
    void setDuration(int32_t duration);

    virtual Variant currentValue() const;
    virtual void setCurrentValue(const Variant &value);

    AnimationCurve *curve(int32_t component = 0) const;
    void setCurve(AnimationCurve *curve, int32_t component = 0);

    void setCurrentTime(uint32_t posintion) override;

protected:
    Quaternion quaternionValue(AnimationCurve *curves[4], float posintion);

private:
    map<int32_t, AnimationCurve *> m_keyFrames;

    Variant m_currentValue;

    int32_t m_duration;

};

#endif // VARIANTANIMATION_H
