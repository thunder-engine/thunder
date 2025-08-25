#ifndef EMITTERSTATE_H
#define EMITTERSTATE_H

#include "effectmodule.h"

class EmitterState : public EffectModule {
    A_OBJECT(EmitterState, EffectModule, Modificator)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    EmitterState();

private:
    void getOperations(std::vector<OperationData> &operations) const override;

};

#endif // EMITTERSTATE_H
