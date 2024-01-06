#ifndef HINGEJOINT_H
#define HINGEJOINT_H

#include "joint.h"

class BULLET_EXPORT HingeJoint : public Joint {
    A_REGISTER(HingeJoint, Joint, General)

    A_NOPROPERTIES()
    A_NOMETHODS()

public:
    HingeJoint();
    ~HingeJoint() override;

};

#endif // HINGEJOINT_H
