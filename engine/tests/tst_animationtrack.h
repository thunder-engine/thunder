#include "gtest/gtest.h"

#include "resources/animationclip.h"

TEST(AnimationTrack, Fix_curves) {
    AnimationTrack track;

    AnimationCurve &curve = track.curve();

    // AnimationTrack::fixCurves must sort frames before normalization
    curve.m_keys.push_back({AnimationCurve::KeyFrame::Linear, 2.0f, {0.0f, 1.0f}});
    curve.m_keys.push_back({AnimationCurve::KeyFrame::Linear, 0.0f, {1.0f, 0.0f}});

    track.fixCurves();

    ASSERT_FLOAT_EQ(1.0f, curve.m_keys.back().m_position);
    ASSERT_FLOAT_EQ(2.0f, track.duration());
}

TEST(AnimationTrack, Sample_Vector4) {
    AnimationTrack track;

    AnimationTrack::Frames &frames = track.frames();
    AnimationCurve &curve = track.curve();

    curve.m_keys.push_back({AnimationCurve::KeyFrame::Linear, 0.0f, {1.0f, 0.0f}});
    curve.m_keys.push_back({AnimationCurve::KeyFrame::Linear, 1.0f, {0.0f, 1.0f}});

    ASSERT_EQ(Vector4(0.5f, 0.5f, 0.0f, 0.0f), track.valueVector4(0.5f));
}

TEST(AnimationTrack, Sample_Quaternion) {
    AnimationTrack track;

    AnimationTrack::Frames &frames = track.frames();
    AnimationCurve &curve = track.curve();

    Quaternion q0;
    Quaternion q1(Vector3(90.0f, 0.0f, 0.0f));

    curve.m_keys.push_back({AnimationCurve::KeyFrame::Linear, 0.0f, {q0.x, q0.y, q0.z, q0.w}});
    curve.m_keys.push_back({AnimationCurve::KeyFrame::Linear, 1.0f, {q1.x, q1.y, q1.z, q1.w}});

    Quaternion sample(track.valueQuaternion(0.5f));
    Quaternion result(Vector3(45.0f, 0.0f, 0.0f));
    ASSERT_TRUE(result.equal(sample));
}

TEST(AnimationTrack, Sample_String) {
    AnimationTrack track;

    AnimationTrack::Frames &frames = track.frames();
    frames.push_back({"test1", 0.0f });
    frames.push_back({"test2", 0.5f });
    frames.push_back({"test3", 1.0f });

    ASSERT_EQ(TString("test1"), track.valueString(0.45f));
    ASSERT_EQ(TString("test2"), track.valueString(0.55f));
    ASSERT_EQ(TString("test3"), track.valueString(1.0f));
}

TEST(AnimationTrack, Serialization) {
    AnimationTrack track;

    const TString path("/test/path");
    const TString property("testProperty");

    track.setPath(path);
    track.setProperty(property);

    AnimationCurve &curve = track.curve();
    curve.m_keys.push_back({AnimationCurve::KeyFrame::Linear, 0.0f, {0.0f, 1.0f, 2.0f, 3.0f}});
    curve.m_keys.push_back({AnimationCurve::KeyFrame::Linear, 0.5f, {1.0f, 2.0f, 3.0f, 0.0f}});
    curve.m_keys.push_back({AnimationCurve::KeyFrame::Linear, 1.0f, {2.0f, 3.0f, 0.0f, 1.0f}});

    track.setDuration(1000);

    AnimationTrack::Frames &frames = track.frames();
    frames.push_back({"test1", 0.0f});
    frames.push_back({"test2", 1.0f});

    Variant data = track.toVariant();

    AnimationTrack dataTrack;
    dataTrack.fromVariant(data);

    ASSERT_EQ(path, dataTrack.path());
    ASSERT_EQ(property, dataTrack.property());

    ASSERT_EQ(1000, dataTrack.duration());

    AnimationCurve &dataCurve = dataTrack.curve();
    ASSERT_EQ(3, dataCurve.m_keys.size());
    ASSERT_FLOAT_EQ(1.0f, dataCurve.m_keys[0].m_value[1]);
    ASSERT_FLOAT_EQ(3.0f, dataCurve.m_keys[1].m_value[2]);
    ASSERT_FLOAT_EQ(2.0f, dataCurve.m_keys[2].m_value[0]);

    AnimationTrack::Frames &dataFrames = dataTrack.frames();
    ASSERT_EQ(2, dataFrames.size());
    ASSERT_EQ(TString("test2"), dataFrames.back().m_value);
}
