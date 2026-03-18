/*
    This file is part of Thunder Next.

    Thunder Next is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Thunder Next is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Thunder Next. If not, see <http://www.gnu.org/licenses/>.

    Copyright: 2008-2026 Evgeniy Prikazchikov
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
