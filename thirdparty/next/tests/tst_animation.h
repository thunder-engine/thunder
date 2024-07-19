#include "tst_common.h"

#include "anim/propertyanimation.h"
#include "anim/variantanimation.h"

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
    k1.m_value = Vector2(0.0f, 0.0f);
    k1.m_position = 0.0f;
    k1.m_type = AnimationCurve::KeyFrame::Linear;
    curve.m_keys.push_back(k1);

    AnimationCurve::KeyFrame k2;
    k2.m_value = Vector2(1.0f, 2.0f);
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
