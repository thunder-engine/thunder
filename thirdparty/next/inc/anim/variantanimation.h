/*
    This file is part of Thunder Next.

    Thunder Next is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

    Thunder Next is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Thunder Next.  If not, see <http://www.gnu.org/licenses/>.

    Copyright: 2008-2023 Evgeniy Prikazchikov
*/

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

    const AnimationCurve &curve() const;
    void setCurve(const AnimationCurve &curve);

    void setCurrentTime(uint32_t posintion) override;

private:
    AnimationCurve m_keyFrames;

    Variant m_currentValue;

    int32_t m_duration;

};

#endif // VARIANTANIMATION_H
