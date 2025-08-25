#include "emitterstate.h"

EmitterState::EmitterState() {
    m_stage = EmitterUpdate;

    setName("EmitterState");
}

void EmitterState::getOperations(std::vector<OperationData> &operations) const {
    operations.push_back({Operation::Subtract, variable("e.age"), {variable("e.age"), variable("s.deltaTime")}});
    operations.push_back({Operation::Max, variable("e.age"), {variable("e.age"), variable("0")}});
}
