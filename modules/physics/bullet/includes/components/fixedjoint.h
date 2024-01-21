#ifndef FIXEDJOINT_H
#define FIXEDJOINT_H

#include "joint.h"

class BULLET_EXPORT FixedJoint : public Joint {
    A_REGISTER(FixedJoint, Joint, Components/Physics)

    A_NOPROPERTIES()
    A_NOMETHODS()

public:
    FixedJoint();

private:
    void createConstraint() override;

};

#endif // FIXEDJOINT_H
