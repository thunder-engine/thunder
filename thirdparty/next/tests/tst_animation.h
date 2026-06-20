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

#include "tst_common.h"

#include "anim/propertyanimation.h"

namespace NextSuite {

    class AnimationTest : public ::testing::Test {

    };

    TEST_F(AnimationTest, Base_animation) {
        Animation anim;
    }

    TEST_F(AnimationTest, Property_animation) {
        PropertyAnimation anim;
        TestObject object;
        anim.setTarget(&object, "vec");
        ASSERT_TRUE((anim.target() != nullptr) == true);

        AnimationCurve curve;

        AnimationCurve::KeyFrame k1;
        k1.m_value = {0.0f, 0.0f};
        k1.m_position = 0.0f;
        k1.m_type = AnimationCurve::KeyFrame::Linear;
        curve.m_keys.push_back(k1);

        AnimationCurve::KeyFrame k2;
        k2.m_value = {1.0f, 2.0f};
        k2.m_position = 1.0f;
        k2.m_type = AnimationCurve::KeyFrame::Linear;
        curve.m_keys.push_back(k2);

        anim.setCurve(curve);

        AnimationCurve curveY;

        anim.setDuration(1000);
        anim.start();

        anim.setCurrentTime(500);

        ASSERT_TRUE(object.getVector() == Vector2(0.5, 1.0f));
    }
}
