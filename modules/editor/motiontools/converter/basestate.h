#ifndef BASESTATE_H
#define BASESTATE_H

#include "entrystate.h"

#include <editor/assetmanager.h>

class AnimationClip;

class BaseState : public StateNode {
    A_OBJECT(BaseState, StateNode, Motion/States)

    A_PROPERTIES(
        A_PROPERTY(string, Name, BaseState::name, BaseState::setName),
        A_PROPERTYEX(AnimationClip *, Clip, BaseState::clip, BaseState::setClip, "editor=Asset"),
        A_PROPERTY(bool, Loop, BaseState::loop, BaseState::setLoop)
    )

public:
    BaseState();

    AnimationClip *clip() const;
    void setClip(AnimationClip *path);

    bool loop() const;
    void setLoop(bool loop);

    Vector2 defaultSize() const override;
    Vector4 color() const override;

private:
    AnimationClip *m_clip;

    bool m_loop;

};

#endif // BASESTATE_H
