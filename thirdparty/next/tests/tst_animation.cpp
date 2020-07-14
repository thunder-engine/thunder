#include "tst_common.h"

#include "anim/propertyanimation.h"
#include "anim/variantanimation.h"

class AnimationTest : public QObject {
    Q_OBJECT
private slots:

void Base_animation() {
    Animation anim;
}

void Variant_animation() {
    VariantAnimation anim;
/*
    {
        VariantAnimation::Curve curve   = { KeyFrame(0,     Variant(0.0f)),
                                            KeyFrame(1000,  Variant(10.0f)) };
        anim.setKeyFrames(curve);
        anim.setCurrentTime(200);
        QCOMPARE(anim.currentValue().toInt(), 2);
    }
    {
        VariantAnimation::Curve curve   = { KeyFrame(0,     Variant(1.0f)),
                                            KeyFrame(1000,  Variant(2.0f)) };
        anim.setKeyFrames(curve);
        anim.setCurrentTime(250);
        QCOMPARE(anim.currentValue().toFloat(), 1.25f);
    }
    {
        VariantAnimation::Curve curve   = { KeyFrame(0,     Variant(Vector2(1.0f, 2.0f))),
                                            KeyFrame(1000,  Variant(Vector2(3.0f, 4.0f))) };

        anim.setKeyFrames(curve);
        anim.setCurrentTime(300);
        QCOMPARE(anim.currentValue().toVector2(), Vector2(1.6f, 2.6f));
    }
    {
        VariantAnimation::Curve curve   = { KeyFrame(0,     Variant(Vector3(1.0f, 2.0f, 3.0f))),
                                            KeyFrame(1000,  Variant(Vector3(3.0f, 4.0f, 5.0f))) };

        anim.setKeyFrames(curve);
        anim.setCurrentTime(300);
        QCOMPARE(anim.currentValue().toVector3(), Vector3(1.6f, 2.6f, 3.6f));
    }
    {
        VariantAnimation::Curve curve   = { KeyFrame(0,     Variant(Vector4(1.0f, 2.0f, 3.0f, 4.0f))),
                                            KeyFrame(500,   Variant(Vector4(4.0f, 5.0f, 6.0f, 7.0f))),
                                            KeyFrame(1000,  Variant(Vector4(3.0f, 4.0f, 5.0f, 6.0f))) };

        anim.setKeyFrames(curve);
        anim.setCurrentTime(500);
        Vector4 v   = anim.currentValue().toVector4();
        QCOMPARE(v, Vector4(4.0f, 5.0f, 6.0f, 7.0f));

        anim.setCurrentTime(750);
        QCOMPARE(anim.currentValue().toVector4(), Vector4(3.5f, 4.5f, 5.5f, 6.5f));
    }
    {
        VariantAnimation::Curve curve   = { KeyFrame(0,     Variant(Quaternion(Vector3(0.0f, 1.0f, 0.0f), 0.0f))),
                                            KeyFrame(1000,  Variant(Quaternion(Vector3(0.0f, 1.0f, 0.0f), 90.0f))) };

        anim.setKeyFrames(curve);
        anim.setCurrentTime(500);

        Quaternion result   = anim.currentValue().toQuaternion();
        Quaternion expected = Quaternion(Vector3(0.0f, 1.0f, 0.0f), 45.0f);
        //QCOMPARE(result, expected);
    }

    {
        VariantAnimation::Curve curve   = { KeyFrame(0,     KeyFrame::Cubic, Variant(0.0f),Variant( 0.0f), Variant(0.0f)),
                                            KeyFrame(500,   KeyFrame::Cubic, Variant(0.0f),Variant(-16.0f),Variant(-16.0f)),
                                            KeyFrame(1000,  KeyFrame::Cubic, Variant(0.0f),Variant( 0.0f), Variant(0.0f)) };

        anim.setKeyFrames(curve);

        anim.setCurrentTime(150);
        float v0 = anim.currentValue().toFloat();

        anim.setCurrentTime(500);
        float v   = anim.currentValue().toFloat();
        QCOMPARE(v, 0.0f);

        anim.setCurrentTime(850);
        float v1 = anim.currentValue().toFloat();

        QCOMPARE(v0, v1);
    }
*/
}

void Property_animation() {
    PropertyAnimation anim;
/*
    TestObject object;
    anim.setTarget(&object, "vec");
    QCOMPARE((anim.target() != nullptr), true);

    VariantAnimation::Curve curve   = { KeyFrame(0,     Variant(Vector2(0.0f, 0.0f))),
                                        KeyFrame(1000,  Variant(Vector2(1.0f, 2.0f))) };

    anim.setKeyFrames(curve);
    anim.setCurrentTime(500);

    QCOMPARE(object.getVector(), Vector2(0.5, 1.0f));
*/
}

} REGISTER(AnimationTest)

#include "tst_animation.moc"
