#ifndef BASESTATE_H
#define BASESTATE_H

#include <editor/graph/statenode.h>

#include <editor/assetmanager.h>

class AnimationClip;

class BaseState : public StateNode {
    A_OBJECT(BaseState, StateNode, Motion)

    A_PROPERTIES(
        A_PROPERTY(TString, Name, BaseState::name, BaseState::setBaseName),
        A_PROPERTYEX(AnimationClip *, Clip, BaseState::clip, BaseState::setClip, "editor=Asset"),
        A_PROPERTY(bool, Loop, BaseState::loop, BaseState::setLoop)
    )

public:
    AnimationClip *clip() const {
        return m_clip;
    }

    void setClip(AnimationClip *clip) {
        m_clip = clip;
    }

    bool loop() const {
        return m_loop;
    }

    void setLoop(bool loop) {
        m_loop = loop;
    }

    void setBaseName(const TString &name) {
        setName(name);

        updateName();
    }

protected:
    Vector2 defaultSize() const override {
        return Vector2(170.0f, 40.0f);
    }

    Vector4 color() const override {
        return Vector4(0.141f, 0.384f, 0.514f, 1.0f);
    }

private:
    AnimationClip *m_clip = nullptr;

    bool m_loop = false;

};

#endif // BASESTATE_H
