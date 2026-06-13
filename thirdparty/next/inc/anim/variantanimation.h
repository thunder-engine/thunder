/*
    This file is part of Thunder Next.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#ifndef VARIANTANIMATION_H
#define VARIANTANIMATION_H

#include "animation.h"

#include "animationcurve.h"

class NEXT_LIBRARY_EXPORT VariantAnimation : public Animation {
    A_OBJECT(VariantAnimation, Animation, Animation)

public:
    VariantAnimation();

    ~VariantAnimation();

    int32_t duration() const override;
    void setDuration(int32_t duration);

    virtual Variant currentValue() const;
    virtual void setCurrentValue(const Variant &value);

    const AnimationCurve &curve() const;
    void setCurve(const AnimationCurve &curve);

    void setCurrentTime(uint32_t posintion) override;

private:
    AnimationCurve m_keyFrames;

    Variant m_currentValue;

    int32_t m_duration;

};

#endif // VARIANTANIMATION_H
