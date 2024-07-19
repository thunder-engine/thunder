#ifndef BASEANIMATIONBLENDER_H
#define BASEANIMATIONBLENDER_H

#include <propertyanimation.h>

class AnimationTrack;

class BaseAnimationBlender : public PropertyAnimation {
public:
    BaseAnimationBlender();

    void setOffset(float offset);

    void setTransitionTime(float time);

    int32_t duration() const override;

    void setCurrentTime(uint32_t position) override;

    void setPreviousTrack(AnimationTrack &track);

    void setPreviousDuration(int32_t duration);

private:
    AnimationTrack *m_previousTrack;

    float m_factor;
    float m_offset;
    float m_transitionTime;

    uint32_t m_previousDuration;

    uint32_t m_previousTime;
};

#endif //BASEANIMATIONBLENDER_H
