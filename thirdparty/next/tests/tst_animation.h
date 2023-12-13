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

    AnimationCurve curveX;

    AnimationCurve::KeyFrame x1;
    x1.m_Value = 0.0f;
    x1.m_Position = 0.0f;
    x1.m_Type = AnimationCurve::KeyFrame::Linear;
    curveX.m_Keys.push_back(x1);

    AnimationCurve::KeyFrame x2;
    x2.m_Value = 1.0f;
    x2.m_Position = 1.0f;
    x2.m_Type = AnimationCurve::KeyFrame::Linear;
    curveX.m_Keys.push_back(x2);

    anim.setCurve(&curveX, 0);

    AnimationCurve curveY;

    AnimationCurve::KeyFrame y1;
    y1.m_Value = 0.0f;
    y1.m_Position = 0.0f;
    y1.m_Type = AnimationCurve::KeyFrame::Linear;
    curveY.m_Keys.push_back(y1);

    AnimationCurve::KeyFrame y2;
    y2.m_Value = 2.0f;
    y2.m_Position = 1.0f;
    y2.m_Type = AnimationCurve::KeyFrame::Linear;
    curveY.m_Keys.push_back(y2);

    anim.setCurve(&curveY, 1);
    anim.setDuration(1000);
    anim.start();

    anim.setCurrentTime(500);

    ASSERT_TRUE(object.getVector() == Vector2(0.5, 1.0f));
}
