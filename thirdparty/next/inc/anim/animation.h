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

#ifndef ANIMATION_H
#define ANIMATION_H

#include <objectsystem.h>

class NEXT_LIBRARY_EXPORT Animation : public Object {
    A_OBJECT(Animation, Object, Animation)

    A_METHODS(
        A_SLOT(Animation::start),
        A_SLOT(Animation::stop),
        A_SLOT(Animation::pause),
        A_SLOT(Animation::resume)
    )

    A_PROPERTIES(
        A_PROPERTY(uint32_t, Time, Animation::currentTime, Animation::setCurrentTime)
    )

public:
    enum State {
        STOPPED,
        RUNNING,
        PAUSED
    };

public:
    Animation();

    ~Animation();

    uint32_t currentTime() const;
    virtual void setCurrentTime(uint32_t msecs);

    int32_t loopCount() const;
    void setLoopCount(int32_t loops);

    uint32_t currentLoop() const;

    uint32_t loopTime() const;

    State state() const;

    virtual int32_t duration() const;

    int32_t totalDuration() const;

    bool isValid() const;
    virtual void setValid(bool valid);

public:
    void start();

    void stop();

    void pause();

    void resume(bool ignore = false);

private:
    Animation::State m_state;

    uint32_t m_currentLoop;

    uint32_t m_currentTime;

    uint32_t m_totalCurrentTime;

    int32_t  m_loopCount;

    bool m_valid;

};

#endif // ANIMATION_H
