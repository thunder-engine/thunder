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
#ifndef SPRINGJOINT_H
#define SPRINGJOINT_H

#include "joint.h"

class BULLET_EXPORT SpringJoint : public Joint {
    A_OBJECT(SpringJoint, Joint, Components/Physics)

    A_PROPERTIES(
        A_PROPERTY(float, spring, SpringJoint::spring, SpringJoint::setSpring),
        A_PROPERTY(float, damper, SpringJoint::damper, SpringJoint::setDamper)
    )
    A_NOMETHODS()

public:
    SpringJoint();

    float damper() const;
    void setDamper(float damper);

    float spring() const;
    void setSpring(float spring);

private:
    void createConstraint() override;

    void drawGizmosSelected() override;

    void updateParams();

private:
    float m_damper;

    float m_spring;

};

#endif // SPRINGJOINT_H
