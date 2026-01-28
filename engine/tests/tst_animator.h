#include "gtest/gtest.h"

#include "components/transform.h"
#include "components/animator.h"

#include "resources/animationstatemachine.h"

class AnimatorTest : public ::testing::Test {
public:
    void SetUp() override {
        {
            AnimationTrack positionTrack;
            positionTrack.setDuration(1000);
            positionTrack.setPath(gTransform);
            positionTrack.setProperty(gPosition);
            {
                AnimationCurve &curve = positionTrack.curve();
                curve.m_keys.push_back({AnimationCurve::KeyFrame::Linear, 0.0f, {0.0f, 0.0f, 0.0f}});
                curve.m_keys.push_back({AnimationCurve::KeyFrame::Linear, 1.0f, {1.0f, 0.0f, 0.0f}});
            }
            idleClip.addAnimationTrack(positionTrack);

            AnimationTrack rotationTrack;
            rotationTrack.setDuration(1000);
            rotationTrack.setPath(gTransform);
            rotationTrack.setProperty(gQuaternion);
            {
                AnimationCurve &curve = rotationTrack.curve();
                Quaternion q0;
                Quaternion q1(Vector3(90.0f, 0.0f, 0.0f));
                curve.m_keys.push_back({AnimationCurve::KeyFrame::Linear, 0.0f, {q0.x, q0.y, q0.z, q0.w}});
                curve.m_keys.push_back({AnimationCurve::KeyFrame::Linear, 1.0f, {q1.x, q1.y, q1.z, q1.w}});
            }
            idleClip.addAnimationTrack(rotationTrack);
        }

        {
            AnimationTrack positionTrack;
            positionTrack.setDuration(1000);
            positionTrack.setPath(gTransform);
            positionTrack.setProperty(gPosition);
            {
                AnimationCurve &curve = positionTrack.curve();
                curve.m_keys.push_back({AnimationCurve::KeyFrame::Linear, 0.0f, {0.0f, 1.0f, 0.0f}});
                curve.m_keys.push_back({AnimationCurve::KeyFrame::Linear, 1.0f, {0.0f, 0.0f, 0.0f}});
            }
            attackClip.addAnimationTrack(positionTrack);

            AnimationTrack rotationTrack;
            rotationTrack.setDuration(1000);
            rotationTrack.setPath(gTransform);
            rotationTrack.setProperty(gQuaternion);
            {
                AnimationCurve &curve = rotationTrack.curve();
                Quaternion q0(Vector3(0.0f, 90.0f, 0.0f));
                Quaternion q1;
                curve.m_keys.push_back({AnimationCurve::KeyFrame::Linear, 0.0f, {q0.x, q0.y, q0.z, q0.w}});
                curve.m_keys.push_back({AnimationCurve::KeyFrame::Linear, 1.0f, {q1.x, q1.y, q1.z, q1.w}});
            }
            attackClip.addAnimationTrack(rotationTrack);
        }

        idleState.m_clip = &idleClip;
        idleState.m_loop = true;
        idleState.m_hash = Mathf::hashString(gIdle);

        attackState.m_clip = &attackClip;
        attackState.m_loop = false;
        attackState.m_hash = Mathf::hashString(gAttack);

        AnimationTransition backToIdle;
        backToIdle.m_targetState = &idleState;
        attackState.m_transitions.push_back(backToIdle);

        AnimationTransition makeAttack;
        makeAttack.m_targetState = &attackState;

        AnimationTransitionCondition condition;
        condition.m_hash = Mathf::hashString(gParamBool);
        condition.m_rule = AnimationTransitionCondition::Equals;
        condition.m_value = true;
        makeAttack.m_conditions.push_back(condition);
        makeAttack.m_duration = 0.5f;

        idleState.m_transitions.push_back(makeAttack);

        stateMachine.m_states.push_back(&idleState);
        stateMachine.m_states.push_back(&attackState);

        stateMachine.m_initialState = &idleState;

        stateMachine.m_variables[Mathf::hashString(gParamX)] =  1.0f;
        stateMachine.m_variables[Mathf::hashString(gParamY)] = -1.0f;
        stateMachine.m_variables[Mathf::hashString(gParamInt)] = 2;
        stateMachine.m_variables[Mathf::hashString(gParamBool)] = true;

        transform.setName(gTransform);

        transform.setParent(&actor);
        animator.setParent(&actor);
    }

    AnimationState *currentState() const {
        return animator.m_currentState;
    }

    AnimationClip *currentClip() const {
        return animator.m_currentClip;
    }

    const std::unordered_map<int, Variant> &currentVariables() const {
        return animator.m_currentVariables;
    }

    const Animator::TargetProperties &targetProperties(int index) const {
        auto it = std::next(animator.m_bindProperties.begin(), index);
        return it->second;
    }

    void process(float delta) {
        animator.process(delta);
    }

    void checkNextState() {
        animator.checkNextState();
    }

    const char *gTransform = "Transform";
    const char *gPosition = "position";
    const char *gQuaternion = "quaternion";
    const char *gParamBool = "paramBool";
    const char *gParamX = "paramX";
    const char *gParamY = "paramY";
    const char *gParamInt = "paramInt";
    const char *gIdle = "idle";
    const char *gAttack = "attack";

    AnimationState idleState;
    AnimationState attackState;

    AnimationClip idleClip;
    AnimationClip attackClip;

    AnimationStateMachine stateMachine;

    Actor actor;
    Animator animator;
    Transform transform;

};

TEST_F(AnimatorTest, StateMachine) {
    animator.setStateMachine(&stateMachine);

    ASSERT_EQ(&stateMachine, animator.stateMachine());
    ASSERT_EQ(stateMachine.initialState(), currentState());
    ASSERT_EQ(&idleClip, currentClip());
    ASSERT_EQ(4, currentVariables().size());
}

TEST_F(AnimatorTest, SwitchState) {
    animator.setStateMachine(&stateMachine);

    ASSERT_EQ(1, targetProperties(0).playbacks.size());
    ASSERT_EQ(true, targetProperties(0).playbacks.front().state->m_loop);

    process(250);
    ASSERT_EQ(Vector3(0.25f, 0.0f, 0.0f), transform.position());

    animator.setState(gAttack);

    ASSERT_EQ(&attackState, currentState());
    ASSERT_EQ(&attackClip, currentClip());

    process(250);
    ASSERT_EQ(Vector3(0.0f, 0.75f, 0.0f), transform.position());
    ASSERT_TRUE(transform.quaternion().equal(Quaternion(Vector3(0.0f, 67.5f, 0.0f))));

    process(750); // 1000 in total
    ASSERT_EQ(0, targetProperties(0).playbacks.size());

    checkNextState(); // Back to Idle after attack
    ASSERT_EQ(&idleState, currentState());
    ASSERT_EQ(&idleClip, currentClip());

    ASSERT_EQ(1, targetProperties(0).playbacks.size());
    ASSERT_EQ(true, targetProperties(0).playbacks.front().state->m_loop);
}

TEST_F(AnimatorTest, CrossFade) {
    animator.setStateMachine(&stateMachine);

    ASSERT_EQ(1, targetProperties(0).playbacks.size());
    ASSERT_EQ(true, targetProperties(0).playbacks.front().state->m_loop);

    process(250);
    ASSERT_EQ(Vector3(0.25f, 0.0f, 0.0f), transform.position());
    ASSERT_TRUE(transform.quaternion().equal(Quaternion(Vector3(22.5f, 0.0f, 0.0f))));

    animator.crossFade(gAttack, 0.5f);
    ASSERT_EQ(2, targetProperties(0).playbacks.size());

    process(250); // 500 in total
    ASSERT_EQ(Vector3(0.5f, 0.0f, 0.0f), transform.position());
    ASSERT_TRUE(transform.quaternion().equal(Quaternion(Vector3(45.0f, 0.0f, 0.0f))));

    process(250); // 750 in total
    ASSERT_EQ(Vector3(0.375f, 0.375f, 0.0f), transform.position());

    process(250); // 1000 in total
    ASSERT_EQ(Vector3(0.0f, 0.5f, 0.0f), transform.position());
    ASSERT_EQ(1, targetProperties(0).playbacks.size());
    ASSERT_EQ(1, targetProperties(1).playbacks.size());

    process(500); // 1500 in total
    ASSERT_EQ(Vector3(0.0f, 0.0f, 0.0f), transform.position());

    checkNextState(); // Back to Idle after attack
    ASSERT_EQ(&idleState, currentState());
    ASSERT_EQ(&idleClip, currentClip());

    ASSERT_EQ(1, targetProperties(0).playbacks.size());
    ASSERT_EQ(true, targetProperties(0).playbacks.front().state->m_loop);
}

TEST_F(AnimatorTest, VariableTransition) {
    animator.setStateMachine(&stateMachine);

    ASSERT_EQ(&idleState, currentState());

    animator.setBool(gParamBool, true);
    checkNextState();

    ASSERT_EQ(&attackState, currentState());
}

TEST_F(AnimatorTest, SetPosition) {
    animator.setStateMachine(&stateMachine);

    process(250); // Do some progress

    animator.setClip(&idleClip, 0.5f); // Override the progress
    ASSERT_EQ(Vector3(0.5f, 0.0f, 0.0f), transform.position());
}
