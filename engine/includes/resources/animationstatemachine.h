/*
    This file is part of Thunder Engine.

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

#ifndef ANIMATIONSTATEMACHINE_H
#define ANIMATIONSTATEMACHINE_H

#include "resource.h"

#include "animationclip.h"

class AnimationState;

namespace EngineSuite {
    class AnimatorTest;
}

class ENGINE_EXPORT AnimationTransitionCondition {
public:
    enum CompareRules {
        Equals = 0,
        NotEquals,
        Greater,
        Less
    };

public:
    bool check(const Variant &value);

    bool operator== (const AnimationTransitionCondition &right) const {
        return m_hash == right.m_hash;
    }

    Variant m_value;

    int m_hash = 0;

    int m_rule = 0;

};

class ENGINE_EXPORT AnimationTransition {
public:
    bool operator== (const AnimationTransition &right) const {
        return m_targetState == right.m_targetState;
    }

    std::vector<AnimationTransitionCondition> m_conditions;

    AnimationState *m_targetState = nullptr;

    float m_duration = 0.0f;

};
typedef std::vector<AnimationTransition> TransitionVector;

class ENGINE_EXPORT AnimationState {
public:
    bool operator== (const AnimationState &right) const {
        return m_hash == right.m_hash;
    }

    TransitionVector m_transitions;

    AnimationClip *m_clip = nullptr;

    int m_hash = 0;

    bool m_loop = false;

};
typedef std::vector<AnimationState *> AnimationStateVector;

class ENGINE_EXPORT AnimationStateMachine : public Resource {
    A_OBJECT(AnimationStateMachine, Resource, Resources)

    A_METHODS(
        A_METHOD(AnimationState *, AnimationStateMachine::findState),
        A_METHOD(AnimationState *, AnimationStateMachine::initialState)
    )
    A_NOPROPERTIES()

public:
    typedef std::unordered_map<int, Variant> VariableMap;

public:
    AnimationStateMachine();

    AnimationState *findState(int hash) const;

    AnimationState *initialState() const;

    const AnimationStateVector &states() const;

    const VariableMap &variables() const;

    void loadUserData(const VariantMap &data) override;

    VariantMap saveUserData() const override;

    static void registerSuper(ObjectSystem *system);

private:
    AnimationTransitionCondition loadCondition(const VariantList &data) const;

private:
    friend class EngineSuite::AnimatorTest;

    AnimationStateVector m_states;

    AnimationStateMachine::VariableMap m_variables;

    AnimationState *m_initialState;

};

#endif // ANIMATIONSTATEMACHINE_H
