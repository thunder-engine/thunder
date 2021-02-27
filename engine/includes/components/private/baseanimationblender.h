#ifndef BASEANIMATIONBLENDER_H
#define BASEANIMATIONBLENDER_H

#include <propertyanimation.h>

class BaseAnimationBlender : public PropertyAnimation {
public:
    BaseAnimationBlender();

    void setOffset(float offset);

    void setTransitionTime(float time);

    int32_t duration() const override;

    void setCurrentTime(uint32_t position) override;

    AnimationCurve *previousCurve(int32_t component) const;

    void setPreviousCurve(AnimationCurve *curve, int32_t component = 0);

    void setPreviousDuration(int32_t duration);

private:
    float mix(float value, int32_t component, float position);

    Quaternion mix(Quaternion &value, float position);

    unordered_map<int32_t, AnimationCurve *> m_PrevCurve;

    float m_Factor;
    float m_Offset;
    float m_TransitionTime;

    uint32_t m_PrevDuration;

    uint32_t m_PreviousTime;
};

#endif //BASEANIMATIONBLENDER_H
